%% RIP DLQR you can paste to firmware
% Parameters (yours)
mp   = 0.063;
lp   = 0.08;
r    = 0.15;
Jp   = 1.79e-5;
Jarm = 3.9e-4;
Kt   = 0.062806525000000;
Km   = 0.062806525000000;
Rm   = 0.206024725847665;
g    = 9.81;

%% 1) Continuous-time linear model about upright
M = Jarm*Jp + Jp*mp*r^2 + Jarm*mp*lp^2;

A = [ 0  0  1  0;
      0  0  0  1;
      0 (mp^2*g*lp^2*r)/M  -(Kt*Km*(Jp+mp*lp^2)/(Rm*M))  0;
      0 (mp*g*lp*(Jarm+mp*r^2))/M  -(Kt*Km*(mp*lp*r)/(Rm*M))  0 ];

B = [ 0;
      0;
      Kt*(Jp+mp*lp^2)/(Rm*M);
      Kt*(mp*lp*r)/(Rm*M) ];

assert(rank(ctrb(A,B))==4,'Not controllable');

%% 2) Discretize for your MCU
Ts = 1/1000;                 % 1 kHz (matches your firmware filters)
sysc = ss(A,B,eye(4),zeros(4,1));
sysd = c2d(sysc, Ts, 'zoh');
Ad = sysd.A;  Bd = sysd.B;

%% 3) Bryson’s rule for Q,R (balancing)
theta_max     = 0.30;  % rad    (arm stay near 0)
alpha_max     = 0.10;  % rad    (tight around upright)
thetaDot_max  = 2.0;   % rad/s
alphaDot_max  = 4.0;   % rad/s
u_max         = 24.0;   % V (your VOLTAGE_LIMIT)

Qb = diag([1/theta_max^2, 1/alpha_max^2, 1/thetaDot_max^2, 1/alphaDot_max^2]);
Rb = 1/u_max^2;

% Gentle tilt: care more about alpha, slightly relax theta_dot
w_alpha = 3.0;  w_thetadot = 0.5;
Q = Qb .* diag([1, w_alpha, w_thetadot, 1]);
R = Rb;

%% 4) DLQR
[Kd,~,eigz] = dlqr(Ad, Bd, Q, R);
disp('Kd (volts per state), u[k] = -Kd * x[k]:');
disp(Kd);
disp('Closed-loop discrete poles:');  disp(eigz.');

%% 6) Print C array to paste into firmware (lqr.c expects u = -Kx, so use K as-is)
fprintf('C array for rip_config.c:\n');
fprintf('float32_t K_matlab[4] = { %.4ff, %.4ff, %.4ff, %.4ff };\n', Kd);
