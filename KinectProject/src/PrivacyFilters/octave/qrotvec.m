% Get a new vector 'v' by rotating vector 'u' using quaternion 'q'. 
function v = qrotvec(u, q)

	q1 = q * quaternion(0, u(1), u(2), u(3)) * conj(q); % for normalised quaternions
	%q1 = q * quaternion(0, u(1), u(2), u(3)) * inv(q); % for non normalised quaternions
	v = zeros(3,1);
	v(1) = q1.i;
	v(2) = q1.j;
	v(3) = q1.k;

endfunction
