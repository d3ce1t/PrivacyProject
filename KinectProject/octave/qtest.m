% Create vectors
v1 = [1; 0; 0];
v2 = [1; 1; 0];
v3 = [0; 1; 0];
v4 = [-1; 1; 0];
v5 = [0; -1; 0];
v6 = [1; 0; 1];
v7 = [0; 0; 1];
v8 = [-1; 0; 0];

% Create quaternions from vectors
q1 = qvec2q(v1, v2);
q2 = qvec2q(v2, v3);
q3 = qvec2q(v3, v4);

q4 = qvec2q(v1, v3);

q5 = qvec2q(v1, v4);
q6 = qvec2q(v2, v4);

q7 = qvec2q(v1, v5);
q8 = qvec2q(v3, v1);

q9 = qvec2q(v6, v7);
q10 = qvec2q(v1, v8);

printf("\nQuaternions q1, q2 and q3 represent the same rotation of 45º respect to the same axis, so they are equal\n\n");
q1, q2, q3

printf("The same with quaternions q4 and q6\n\n");
q4, q6

printf("And with quaternions q7 and q8, despite of they are created from opposit vectors v1 to v5 and v3 to v1\n\n");
q7, q8

printf("However, quaternion q9 is distinct from q1, q2 and q3 despite of they represent the same rotation, but in respect of a different axis\n\n");
q9

% Distances using qdist1
printf("Distances using qdist1\n\n");
d11 = qdist1(q1, q1)
d12 = qdist1(q1, q2)
d13 = qdist1(q1, q3)
d16 = qdist1(q1, q6)
d23 = qdist1(q2, q3)
d46 = qdist1(q4, q6)
d78 = qdist1(q7, q8)
d19 = qdist1(q1, q9)
d110 = qdist1(q1, q10)

% Distances using qdist2
printf("\nDistances using qdist2\n\n");
d11 = qdist2(q1, q1)
d12 = qdist2(q1, q2)
d13 = qdist2(q1, q3)
d16 = qdist2(q1, q6)
d23 = qdist2(q2, q3)
d46 = qdist2(q4, q6)
d78 = qdist2(q7, q8)
d19 = qdist2(q1, q9)
d110 = qdist2(q1, q10)

% Distances using qdist3
printf("\nDistances using qdist3\n\n");
d11 = qdist3(q1, q1)
d12 = qdist3(q1, q2)
d13 = qdist3(q1, q3)
d16 = qdist3(q1, q6)
d23 = qdist3(q2, q3)
d46 = qdist3(q4, q6)
d78 = qdist3(q7, q8)
d19 = qdist3(q1, q9)
d110 = qdist3(q1, q10)

% Distances using qdist4
printf("\nDistances using qdist4\n\n");
d11 = qdist4(q1, q1)
d12 = qdist4(q1, q2)
d13 = qdist4(q1, q3)
d16 = qdist4(q1, q6)
d23 = qdist4(q2, q3)
d46 = qdist4(q4, q6)
d78 = qdist4(q7, q8)
d19 = qdist4(q1, q9)
d110 = qdist4(q1, q10)

% Distances using qdist5
printf("\nDistances using qdist5\n\n");
d11 = qdist5(q1, q1)
d12 = qdist5(q1, q2)
d13 = qdist5(q1, q3)
d16 = qdist5(q1, q6)
d23 = qdist5(q2, q3)
d46 = qdist5(q4, q6)
d78 = qdist5(q7, q8)
d19 = qdist5(q1, q9)
d110 = qdist5(q1, q10)

% Distances using qdot
printf("\nDistances using qdot\n\n");
d11 = qdot(q1, q1)
d12 = qdot(q1, q2)
d13 = qdot(q1, q3)
d16 = qdot(q1, q6)
d23 = qdot(q2, q3)
d46 = qdot(q4, q6)
d78 = qdot(q7, q8)
d19 = qdot(q1, q9)
d110 = qdot(q1, q10)
