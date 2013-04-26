% Gets a quaternion 'q' which denotes a rotation of 'theta' rads around 'vv' axis.
function q = qrot2q (vv, theta)

	if (abs (norm (vv) - 1) > 1e-12)
	    warning ("quaternion: ||vv|| != 1, normalizing")
	    vv = vv / norm (vv);
	endif

	w = cos(theta/2);
	vv = vv * sin(theta/2) ;
	q = quaternion(w, vv(1), vv(2), vv(3));
	q = unit(q);

endfunction
