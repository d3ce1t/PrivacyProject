% Same as qdist2 but without trigonometric functions.
% Gives 0 when same orientation and 1 when 180º apart.
function distance = qdist3(q1, q2)

	inner_product = qdot(q1, q2);
	distance = 1 - inner_product^2;

endfunction
