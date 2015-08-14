% Distance between quaternions will correspond roughly to distance between
% orientations as long as the quaternions are fairly close to each other.
% However, if you're comparing quaternions globally, you should remember
% that q and −q always represent the same orientation, even though the
% distance between them is 2.
function distance = qdist1(q1, q2)

	distance = norm(q1 - q2);

endfunction
