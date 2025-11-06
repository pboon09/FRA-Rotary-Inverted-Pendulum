clc; clear; close all;

function [Ad, Bd, Kd] = rip_dlqr_gain()
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
    Jarm = I1_com_val;    % Arm moment of inertia about COM
    Jp   = I2_com_val;    % Pendulum moment of inertia about COM
    mp   = m2_val;        % Pendulum mass
    r    = L_pivot_val;   % Distance pivot to arm rotation point (motor → pendulum pivot)
    lp   = L2_com_val;    % Distance pivot to pendulum COM
    g    = g_val;
    
    M = Jarm*Jp + Jp*mp*r^2 + Jarm*mp*lp^2;
    
    A = [0 0 1 0;
         0 0 0 1;
         0 (mp^2*g*lp^2*r)/M -(Kt*Km*(Jp+mp*lp^2)/(Rm*M)) 0;
         0 (mp*g*lp*(Jarm+mp*r^2))/M -(Kt*Km*(mp*lp*r)/(Rm*M)) 0];
    
    B = [0;
         0;
         (Kt*(Jp+mp*lp^2))/(Rm*M);
         (Kt*(mp*lp*r))/(Rm*M)];
    
    Ts = 1.0 / 1000.0;
    sysd = c2d(ss(A,B,eye(4),zeros(4,1)), Ts);
    Ad = sysd.A;
    Bd = sysd.B;
    
    Q = diag([10 600 1 10]);
    R = 0.05;
    [Kd,~,~] = dlqr(Ad,Bd,Q,R);
    
    fprintf('Ad matrix:\n');
    disp(Ad);
    fprintf('Bd vector:\n');
    disp(Bd);
    fprintf('float32_t K_matlab[4] = {');
    fprintf('%+.4ff, ', Kd(1:end-1));
    fprintf('%+.4ff};\n', Kd(end));
end

[Ad, Bd, Kd] = rip_dlqr_gain();