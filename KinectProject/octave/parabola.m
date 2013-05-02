function vector = parabola(a, b, c, x)

	vector =  (a*x.^2 + b*x +c);
	%vector = x .* cosd(90) + vector .* sind(90);

endfunction
