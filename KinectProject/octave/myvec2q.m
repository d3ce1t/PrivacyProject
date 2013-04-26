function q = myvec2q (v, u)
	
	theta = vangle(v, u)*180/pi;
	scalarPart = (norm(v) / norm(u)) * cosd(theta)
	rot_axis = cross(v, u) / norm(cross(v, u));
	vectorPart = rot_axis * (norm(v) / norm(u)) * sind(theta)
	q = quaternion(scalarPart, vectorPart(1), vectorPart(2), vectorPart(3));	

endfunction
