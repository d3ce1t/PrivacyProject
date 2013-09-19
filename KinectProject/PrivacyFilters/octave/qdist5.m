% Distance as used in Human Action Recognition Using Dynamic Time Warping,
% Samsu Sempena, 2011
function distance = qdist5(q1, q2)

	q_sub = q1 - q2;
	q_add = q1 + q2;
	distance = min( sqrt(qdot(q_sub, q_sub)), sqrt(qdot(q_add, q_add)) );

endfunction
