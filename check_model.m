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

M = Jarm*Jp + Jp*mp*r^2 + Jarm*mp*lp^2;

A = [0 0 1 0;
     0 0 0 1;
     0 (mp^2*g*lp^2*r)/M -(Kt*Km*(Jp+mp*lp^2)/(Rm*M)) 0;
     0 (mp*g*lp*(Jarm+mp*r^2))/M -(Kt*Km*(mp*lp*r)/(Rm*M)) 0];

B = [0;
     0;
     Kt*(Jp+mp*lp^2)/(Rm*M);
     Kt*(mp*lp*r)/(Rm*M)];

C = [1 0 0 0;
     0 1 0 0];

D = [0; 0];

Co = ctrb(A,B);
rCo = rank(Co);
fprintf('Rank of controllability matrix: %d (of %d)\n', rCo, size(A,1));

if rCo == size(A,1)
    disp('→ System is fully CONTROLLABLE');
else
    disp('→ System is NOT controllable');
end

Ob = obsv(A,C);
rOb = rank(Ob);
fprintf('Rank of observability matrix: %d (of %d)\n', rOb, size(A,1));

if rOb == size(A,1)
    disp('→ System is fully OBSERVABLE');
else
    disp('→ System is NOT observable');
end

disp('Open-loop poles:');
disp(eig(A));
