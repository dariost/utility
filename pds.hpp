/*****************************************************
 *                                                   *
 * License: Apache License 2.0                       *
 * Author: Dario Ostuni <another.code.996@gmail.com> *
 *                                                   *
 ****************************************************/

#pragma once

#include <cstdint>
#include <queue>
#include <random>
#include <vector>

namespace ostuni {

using namespace std;

typedef int64_t i64;
typedef uint64_t u64;

class iVec2 {
  public:
    i64 x;
    i64 y;
    iVec2(i64 _x = 0, i64 _y = 0) : x(_x), y(_y) {}
};

vector<iVec2> pds(iVec2 ll, iVec2 ur, double radius = 5.0, u64 seed = 0,
                  u64 k_tries = 30) {
    double factor = sqrt(2.0) / 2.0;
    i64 edge_size = radius * factor;
    i64 height = (ur.y - ll.y) / edge_size + 1;
    i64 width = (ur.x - ll.x) / edge_size + 1;
    vector<iVec2> points;
    vector<vector<i64>> grid(height, vector<i64>(width, -1));
    mt19937_64 r(seed);
    uniform_int_distribution<i64> gen_x(ll.x, ur.x - 1);
    uniform_int_distribution<i64> gen_y(ll.y, ur.y - 1);
    auto gen_point = [&r, &gen_x, &gen_y]() {
        return iVec2(gen_x(r), gen_y(r));
    };
    auto map_point = [&ur, &ll, &edge_size](iVec2 p) -> pair<i64, i64> {
        p.x -= ll.x;
        p.y -= ll.y;
        p.x /= edge_size;
        p.y /= edge_size;
        return {p.y, p.x};
    };
    queue<iVec2> q;
    auto starting_point = gen_point();
    auto [sy, sx] = map_point(starting_point);
    grid[sy][sx] = 0;
    points.push_back(starting_point);
    q.push(starting_point);
    uniform_real_distribution<double> gen_radius(radius, radius * 2.0);
    uniform_real_distribution<double> gen_theta(0.0, M_PI * 2.0);
    while(!q.empty()) {
        auto p = q.front();
        q.pop();
        bool at_least_one = false;
        for(u64 t = 0; t < k_tries; t++) {
            double radius = gen_radius(r);
            double theta = gen_theta(r);
            i64 dx = radius * cos(theta);
            i64 dy = radius * sin(theta);
            iVec2 np(p.x + dx, p.y + dy);
            if(np.x < ll.x || np.y < ll.y || np.x >= ur.x || np.y >= ur.y) {
                continue;
            }
            auto [ny, nx] = map_point(np);
            if(ny < 0 || ny >= height || nx < 0 || nx >= width) {
                continue;
            }
            if(grid[ny][nx] >= 0) {
                continue;
            }
            bool ok = true;
            for(i64 y = max(ny - 1, 0L); y <= min(ny + 1, height - 1) && ok;
                y++) {
                for(i64 x = max(nx - 1, 0L); x <= min(nx + 1, width - 1) && ok;
                    x++) {
                    if(grid[y][x] >= 0) {
                        auto op = points[grid[y][x]];
                        if(hypot(op.x - np.x, op.y - np.y) <= radius) {
                            ok = false;
                        }
                    }
                }
            }
            if(!ok) {
                continue;
            }
            grid[ny][nx] = points.size();
            points.push_back(np);
            q.push(np);
            at_least_one = true;
        }
        if(at_least_one) {
            q.push(p);
        }
    }
    return points;
}

} // namespace ostuni
