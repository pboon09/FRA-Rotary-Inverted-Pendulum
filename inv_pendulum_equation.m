%% Dynamic Modeling of Rotary Inverted Pendulum (RIP) - Lagrange Method
clear; clc;
% 1. กำหนดสัญลักษณ์
% q1 = theta (แขนหมุน), q2 = beta (ลูกตุ้ม)
syms q1 q2 dq1 dq2 ddq1 ddq2 real % พิกัดทั่วไป, ความเร็ว, ความเร่ง
syms m1 m2 I1 I2 L1 L2 g real % พารามิเตอร์ทางกายภาพ
syms tau tau1 tau2 b1 b2 real % แรงควบคุม, แรงเสียดทาน

% กำหนดค่าพารามิเตอร์
m1_val = 217.21*1e-3;
m2_val = 62.00*1e-3;
L1_val = 240.40*1e-3;
L2_val = 255.46*1e-3;
I1_val = 1285700.06*1e-9;
I2_val = 4177960.34*1e-9;
g_val = 9.81; % ความเร่งโน้มถ่วง

% 2. Kinematics: ตำแหน่ง CoM ของแต่ละ Link ใน World Frame {0}
x1 = L1*cos(q1);
y1 = L1*sin(q1);
z1 = 0;

x2 = L1*cos(q1) - L2*sin(q2)*sin(q1);
y2 = L1*sin(q1) + L2*sin(q2)*cos(q1);
z2 = L2*cos(q2);

% 3. Kinematics: ความเร็ว CoM (ใช้ Chain Rule)
dx1 = diff(x1, q1) * dq1;
dy1 = diff(y1, q1) * dq1;
dz1 = 0;

dx2 = diff(x2, q1) * dq1 + diff(x2, q2) * dq2;
dy2 = diff(y2, q1) * dq1 + diff(y2, q2) * dq2;
dz2 = diff(z2, q2) * dq2;

% 4. พลังงานจลน์ (Kinetic Energy - T)
T = 0.5*m1*(dx1^2 + dy1^2 + dz1^2) + 0.5*I1*dq1^2 + ...
    0.5*m2*(dx2^2 + dy2^2 + dz2^2) + 0.5*I2*dq2^2;

% 5. พลังงานศักย์ (Potential Energy - V)
V = m2*g*z2;

% 6. Lagrangian (L)
L = T - V;

% 7. พิกัดทั่วไปและแรงทั่วไป
q = [q1; q2];
dq = [dq1; dq2];
Q = [tau1; tau2];

% 8. สมการ Euler-Lagrange (EOM)
EOM = sym(zeros(length(q), 1));
for i = 1:length(q)
    L_dq = diff(L, dq(i));
    L_dq_dt = diff(L_dq, q1) * dq1 + ...
              diff(L_dq, q2) * dq2 + ...
              diff(L_dq, dq1) * ddq1 + ...
              diff(L_dq, dq2) * ddq2;
    L_q = diff(L, q(i));
    EOM(i) = simplify(L_dq_dt - L_q - Q(i));
end

% แทนค่าตัวเลขลงในสมการ
EOM_numeric = subs(EOM, [m1, m2, L1, L2, I1, I2, g], ...
                        [m1_val, m2_val, L1_val, L2_val, I1_val, I2_val, g_val]);

% แสดงผลแบบตัวเลข
disp('========================================');
disp('Equation of Motion for q1 (Rotary Arm):');
disp('========================================');
EOM1_numeric = simplify(EOM_numeric(1));
fprintf('0 = %s\n\n', char(vpa(EOM1_numeric, 6)));

disp('========================================');
disp('Equation of Motion for q2 (Pendulum):');
disp('========================================');
EOM2_numeric = simplify(EOM_numeric(2));
fprintf('0 = %s\n\n', char(vpa(EOM2_numeric, 6)));

% แสดงสมการในรูปแบบ M*ddq + C*dq + G = Q
disp('========================================');
disp('Matrix Form: M(q)*ddq + C(q,dq) + G(q) = Q');
disp('========================================');

% สกัดเมทริกซ์มวล M
M = sym(zeros(2,2));
M(1,1) = diff(EOM_numeric(1), ddq1);
M(1,2) = diff(EOM_numeric(1), ddq2);
M(2,1) = diff(EOM_numeric(2), ddq1);
M(2,2) = diff(EOM_numeric(2), ddq2);

disp('Mass Matrix M(q):');
disp(vpa(M, 6));

% สกัดพจน์ Coriolis และ Centrifugal
C_terms = sym(zeros(2,1));
for i = 1:2
    temp = EOM_numeric(i) + Q(i);
    temp = temp - M(i,1)*ddq1 - M(i,2)*ddq2;
    C_terms(i) = temp;
end

disp('Coriolis + Centrifugal + Gravity Terms:');
disp(vpa(C_terms, 6));