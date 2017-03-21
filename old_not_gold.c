// Eulerian method
tracked_points[tracker_i].vx = (tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_k]) / dt1;
tracked_points[tracker_i].vy = (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt1;
vx_prev = (tracked_points[tracker_i].x[prev_k] - tracked_points[tracker_i].x[prev_prev_k]) / dt2;
vy_prev = (tracked_points[tracker_i].y[prev_k] - tracked_points[tracker_i].y[prev_prev_k]) / dt2;
tracked_points[tracker_i].ax = (tracked_points[tracker_i].vx - vx_prev) / dt1;
tracked_points[tracker_i].ay = (tracked_points[tracker_i].vy - vy_prev) / dt1;

// Verlet method: compute v at time prev_k
tracked_points[tracker_i].vx =
    (tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_prev_k]) / (dt1 + dt2);
tracked_points[tracker_i].vy =
    (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_prev_k]) / (dt1 + dt2);

tracked_points[tracker_i].ax =
    ((tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_k]) / dt1 +
     (tracked_points[tracker_i].x[prev_prev_k] - tracked_points[tracker_i].x[prev_k]) / dt2)
    * 2 / (dt1 + dt2);
tracked_points[tracker_i].ay =
    ((tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt1 +
     (tracked_points[tracker_i].y[prev_prev_k] - tracked_points[tracker_i].y[prev_k]) / dt2)
    * 2 / (dt1 + dt2);

// Base base
tracked_points[tracker_i].vx = (tracked_points[tracker_i].x[k] - tracked_points[tracker_i].x[prev_k]) / dt;
pred_x = tracked_points[tracker_i].vx * dt + tracked_points[tracker_i].x[k];
tracked_points[tracker_i].vy = (tracked_points[tracker_i].y[k] - tracked_points[tracker_i].y[prev_k]) / dt;
pred_y = (1 / 2 * tracked_points[tracker_i].ay * dt + tracked_points[tracker_i].vy) * dt + tracked_points[tracker_i].y[k];