clc; clear; close all;

% Parameters
m1_val = 220.54e-3;      % Arm mass (kg)
m2_val = 62.00e-3;       % Pendulum mass (kg)
L1_com_val = 35.402e-3;  % Arm COM length from pivot (m)
L2_com_val = 55.75e-3;   % Pendulum COM length from pivot (m)
L_pivot_val = 133e-3;    % Distance from motor to pendulum pivot (m)
I1_com_val = 769047.65e-9;   % Arm inertia about COM (kg·m²)
I2_com_val = 130015.24e-9;   % Pendulum inertia about COM (kg·m²)
g_val  = 9.81;            % Gravity (m/s²)

Kt = 0.062806525000000;   % Motor torque constant (N·m/A)
Km = 0.062806525000000;   % Back EMF constant (V·s/rad)
Rm = 0.206024725847665;   % Armature resistance (Ω)

% Derived parameters
Jarm = I1_com_val + m1_val * L1_com_val^2;   % (kg·m²)
Jp   = I2_com_val;                           % Pendulum inertia about COM (kg·m²)
mp   = m2_val;                               % Pendulum mass (kg)
r    = L_pivot_val;                          % Arm length to pendulum pivot (m)
lp   = L2_com_val;                           % Pendulum COM length (m)
g    = g_val;

%Continuous-time linear model about upright
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

%Discretize
Ts = 1/1000;
sysc = ss(A,B,eye(4),zeros(4,1));
sysd = c2d(sysc, Ts, 'zoh');
Ad = sysd.A;  Bd = sysd.B;

%Bryson’s rule
theta_max     = 0.10;  % rad    (arm stay near 0)
alpha_max     = 0.10;  % rad    (tight around upright)
thetaDot_max  = 1.0;   % rad/s
alphaDot_max  = 1.0;   % rad/s
u_max         = 24.0;   % V (Limit)

Qb = diag([1/theta_max^2, 1/alpha_max^2, 1/thetaDot_max^2, 1/alphaDot_max^2]);
Rb = 1/u_max^2;

% Gentle tilt: care more about alpha, slightly relax theta_dot
w_alpha = 3.0;  w_thetadot = 0.5;
Q = Qb .* diag([1, w_alpha, w_thetadot, 1]);
R = Rb;

[Kd,~,eigz] = dlqr(Ad, Bd, Q, R);
disp('Kd (volts per state), u[k] = -Kd * x[k]:');
disp(Kd);
disp('Closed-loop discrete poles:');  disp(eigz.');

fprintf('C array for rip_config.c:\n');
fprintf('float32_t K_matlab[4] = { %.4ff, %.4ff, %.4ff, %.4ff };\n', Kd);
