function q = vec2q (v, u)

	v_unit = v / norm(v);
	u_unit = u / norm(u);
	k_cos_theta = dot(v_unit, u_unit);
	%k = sqrt(norm(v)^2 * norm(u)^2);
	k = 1;

	if (k_cos_theta / k != -1)
		scalarPart = k + k_cos_theta;
		vectorPart = cross(v_unit, u_unit);
		q = quaternion(scalarPart, vectorPart(1), vectorPart(2), vectorPart(3));	
	else
		q = quaternion(0, 0, 1, 0);
	endif

	q = unit(q);

% v, u, r = rotvec(v, q), rn = norm(r), normal = norm(v)*norm(u), runit = r / norm(r), runit*(normal / rn)

endfunction
