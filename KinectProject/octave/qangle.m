function theta = qangle(q)

	s = q.s;
	vv = [q.x, q.y, q.z];
	theta = acos (s) * 2;

	if (abs (theta) > pi)
		theta = theta - sign (theta) * pi;
	endif

endfunction
