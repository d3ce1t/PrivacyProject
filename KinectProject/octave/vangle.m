% Gets the 'theta' angle in rads between vectors 'v' and 'u' respect the
% imaginary axes defined by cross(v, u).
function theta = vangle(v, u)

	% theta = acos( dot(v, u) / (norm(v) * norm(u))  );
	theta = atan2( norm(cross(v,u)), dot(v,u) ); % optimization

endfunction
