% Gives the angle between two quaternions in rads, between 0 and pi
function theta = qdist2(q1, q2)

	inner_product = qdot(q1, q2);
	theta = acos( 2*(inner_product)^2  -  1 ); 

endfunction
