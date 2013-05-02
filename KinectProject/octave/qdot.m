% Dot product or inner product
function dot_product = qdot(q1, q2)

	dot_product = q1.s * q2.s + q1.i * q2.i + q1.j * q2.j + q1.k * q2.k;

endfunction
