function distance = qdist4(q1, q2)

	q = q1 - q2;
	distance = sqrt(qdot(q, q));

endfunction
