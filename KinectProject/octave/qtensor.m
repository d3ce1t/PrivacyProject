function tensor = qtensor(q)

	vv = zeros(3, 1);
	vv(1) = q.i;
	vv(2) = q.j;
	vv(3) = q.k;

	theta = qangle(q);
	vu = vv / norm(vv);

	found = 0;
	i = 1;

	while (!found && i <= 3)
		if (vv(i) != 0)
			k = vv(i) / vu(i);
			found = 1;
		endif
		i = i+1;
	endwhile
	
	norm_v_norm_u = k / sin(theta);
	tensor = norm_v_norm_u;

endfunction
