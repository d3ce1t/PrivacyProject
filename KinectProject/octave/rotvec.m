function v = rotvec(u, q)

	q1 = q * quaternion(0, u(1), u(2), u(3)) * conj(q);
	v = zeros(3,1);
	v(1) = q1.i;
	v(2) = q1.j;
	v(3) = q1.k;

endfunction
