function theta = qangle(q)

	if (q.s > 1)
		q = unit(q);
	endif

	s = q.s;
	vv = [q.x, q.y, q.z];
	theta = acos (s) * 2;

	% Set in the range 0 - pi
	%if (abs (theta) > pi)
	%	theta = theta - sign (theta) * pi;
	%endif

endfunction
