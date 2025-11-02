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
