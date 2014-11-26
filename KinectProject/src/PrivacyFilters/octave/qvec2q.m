% Gets the quaternion that express the rotation of vector 'v' to be orientated in the same
% orientation that vector 'u'. For whatever reason, the scale isn't performed.
function q = qvec2q (v, u)

	k_cos_theta = dot(v, u); % k * cos(theta) = norm(vv)*norm(uu)*cos(theta)
	k = sqrt( normsquared(v)*normsquared(u) );

	if (k_cos_theta / k != -1)
		scalarPart = k + k_cos_theta;
		vectorPart = cross(v, u);
		q = quaternion(scalarPart, vectorPart(1), vectorPart(2), vectorPart(3));
	else
		q = quaternion(0, 0, 1, 0);
	endif

	q = unit(q);

endfunction
