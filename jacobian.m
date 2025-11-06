%% Rotary Inverted Pendulum - Jacobian Calculation
clear; clc;

% Parameters
L0 = 64e-3;   % Base height
L1 = 133e-3; % Arm length
L2 = 135e-3; % Pendulum CoM length

% Configuration (only 2 actuated joints)
theta = 0.0;  % Arm rotation (joint 1)
beta = 0.0;   % Pendulum angle (joint 2)

% MDH Transform function
function T = mdh_transform(a, alpha, d, theta)
    ca = cos(alpha); sa = sin(alpha);
    ct = cos(theta); st = sin(theta);
    T = [ct,    -st,    0,      a;
         st*ca,  ct*ca, -sa,   -sa*d;
         st*sa,  ct*sa,  ca,    ca*d;
         0,      0,      0,      1];
end

% Build transformations (3 links + tool)
% Link 1: Base rotation (actuated)
T01 = mdh_transform(0, 0, L0, theta);

% Link 2: Dummy link (fixed at pi/2)
T12 = mdh_transform(L1, pi/2, 0, pi/2);  % Fixed offset

% Link 3: Pendulum rotation (actuated)
T23 = mdh_transform(0, pi/2, 0, beta);

% Tool transform: Extension to CoM
T3e = [1, 0, 0, L2;
       0, 1, 0, 0;
       0, 0, 1, 0;
       0, 0, 0, 1];

% Forward kinematics
T02 = T01 * T12;
T03 = T02 * T23;
T0e = T03 * T3e;  % End effector (CoM)

% Extract rotation axes (z-axes of each frame)
z0 = [0; 0; 1];        % Base frame z-axis
z1 = T01(1:3, 3);      % Frame 1 z-axis (joint 1 rotation axis)
z2 = T02(1:3, 3);      % Frame 2 z-axis (dummy, not used)
z3 = T03(1:3, 3);      % Frame 3 z-axis (joint 2 rotation axis)

% Extract positions
p0 = [0; 0; 0];        % Base position
p1 = T01(1:3, 4);      % Joint 1 position
p2 = T02(1:3, 4);      % Dummy joint position
p3 = T03(1:3, 4);      % Joint 2 position
pe = T0e(1:3, 4);      % End effector position

% Calculate Jacobian (6x3 for 3 joints)
J_full = zeros(6, 3);

% Joint 1 (theta) - Revolute
J_full(1:3, 1) = cross(z1, pe - p1);  % Linear velocity
J_full(4:6, 1) = z1;                   % Angular velocity

% Joint 2 (dummy) - Fixed, but still compute for completeness
J_full(1:3, 2) = cross(z2, pe - p2);  % Linear velocity
J_full(4:6, 2) = z2;                   % Angular velocity

% Joint 3 (beta) - Revolute
J_full(1:3, 3) = cross(z3, pe - p3);  % Linear velocity
J_full(4:6, 3) = z3;                   % Angular velocity

fprintf('============================================\n');
fprintf('FULL JACOBIAN [6x3]:\n');
fprintf('============================================\n');
fprintf('         Joint1    Dummy     Joint2\n');
fprintf('         (theta)   (fixed)   (beta)\n');
fprintf('--------------------------------------\n');
labels = {'vx  ', 'vy  ', 'vz  ', 'wx  ', 'wy  ', 'wz  '};
for i = 1:6
    fprintf('%s  %8.5f  %8.5f  %8.5f\n', labels{i}, ...
            J_full(i,1), J_full(i,2), J_full(i,3));
end

% Remove dummy joint (column 2)
J_actuated = J_full(:, [1, 3]);

fprintf('\n============================================\n');
fprintf('ACTUATED JOINTS JACOBIAN [6x2]:\n');
fprintf('============================================\n');
fprintf('         theta     beta\n');
fprintf('         (q1)      (q2)\n');
fprintf('--------------------------------------\n');
for i = 1:6
    fprintf('%s  %8.5f  %8.5f\n', labels{i}, ...
            J_actuated(i,1), J_actuated(i,2));
end

fprintf('\n--- Matrix Notation ---\n');
fprintf('       r11       r12\n');
fprintf('       r21       r22\n');
fprintf('       r31       r32\n');
fprintf('       r41       r42\n');
fprintf('       r51       r52\n');
fprintf('       r61       r62\n');
fprintf('\nActual values:\n');
for i = 1:6
    fprintf('r%d1 = %9.6f   r%d2 = %9.6f\n', ...
            i, J_actuated(i,1), i, J_actuated(i,2));
end

% Physical interpretation
fprintf('\n============================================\n');
fprintf('PHYSICAL INTERPRETATION:\n');
fprintf('============================================\n\n');
fprintf('Joint 1 (THETA) - Arm Rotation:\n');
fprintf('  Rotation axis z1 = [%6.3f, %6.3f, %6.3f]\n', z1(1), z1(2), z1(3));
fprintf('  Effect on end-effector:\n');
fprintf('    Linear:  [%7.4f, %7.4f, %7.4f]\n', J_actuated(1,1), J_actuated(2,1), J_actuated(3,1));
fprintf('    Angular: [%7.4f, %7.4f, %7.4f]\n\n', J_actuated(4,1), J_actuated(5,1), J_actuated(6,1));

fprintf('Joint 2 (BETA) - Pendulum Swing:\n');
fprintf('  Rotation axis z3 = [%6.3f, %6.3f, %6.3f]\n', z3(1), z3(2), z3(3));
fprintf('  Effect on end-effector:\n');
fprintf('    Linear:  [%7.4f, %7.4f, %7.4f]\n', J_actuated(1,2), J_actuated(2,2), J_actuated(3,2));
fprintf('    Angular: [%7.4f, %7.4f, %7.4f]\n\n', J_actuated(4,2), J_actuated(5,2), J_actuated(6,2));

% Test at different configuration
fprintf('============================================\n');
fprintf('TEST AT theta=45°, beta=30°:\n');
fprintf('============================================\n');
theta_test = deg2rad(45);
beta_test = deg2rad(30);

T01_t = mdh_transform(0, 0, L0, theta_test);
T12_t = mdh_transform(L1, pi/2, 0, pi/2);
T23_t = mdh_transform(0, -pi/2, 0, beta_test);
T0e_t = T01_t * T12_t * T23_t * T3e;
T03_t = T01_t * T12_t * T23_t;

z1_t = T01_t(1:3, 3);
z3_t = T03_t(1:3, 3);

pe_t = T0e_t(1:3, 4);
p1_t = T01_t(1:3, 4);
p3_t = T03_t(1:3, 4);

J_test = zeros(6, 2);
J_test(1:3, 1) = cross(z1_t, pe_t - p1_t);
J_test(4:6, 1) = z1_t;
J_test(1:3, 2) = cross(z3_t, pe_t - p3_t);
J_test(4:6, 2) = z3_t;

fprintf('\nEnd-effector position: [%7.4f, %7.4f, %7.4f]\n', pe_t(1), pe_t(2), pe_t(3));
fprintf('\nJacobian at theta=45°, beta=30°:\n');
for i = 1:6
    fprintf('%s  %8.5f  %8.5f\n', labels{i}, J_test(i,1), J_test(i,2));
end