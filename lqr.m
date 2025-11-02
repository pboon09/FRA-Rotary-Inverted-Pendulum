function [Ad, Bd, Kd] = rip_dlqr_gain()
    mp   = 0.063;
    lp   = 0.08;
    r    = 0.15;
    Jp   = 1.79e-5;
    Jarm = 3.9e-4;
    Kt   = 0.062806525000000;
    Km   = 0.062806525000000;
    Rm   = 0.206024725847665;
    g    = 9.81;
    
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