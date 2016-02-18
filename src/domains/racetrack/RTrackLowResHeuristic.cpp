#include <vector>
#include <cassert>

#include "../../../include/solvers/Solver.h"
#include "../../../include/solvers/VISolver.h"

#include "../../../include/domains/racetrack/RacetrackProblem.h"
#include "../../../include/domains/racetrack/RTrackLowResHeuristic.h"

RTrackLowResHeuristic::RTrackLowResHeuristic(char* filename,
                                             int resolution,
                                             double pSlip,
                                             double pError)
{
    lowResProblem_ = new RacetrackProblem(filename);
    std::vector< std::vector <char> > originalTrack = lowResProblem_->track();
    std::vector< std::vector <char> > lrTrack;
    IntPairSet starts;
    IntPairSet goals;

    lrTrack.push_back(std::vector<char> ());
    for (int j = 0; j < originalTrack[0].size(); j += resolution)
        lrTrack.back().push_back(rtrack::wall);
    lrTrack.back().push_back(rtrack::wall);
    lrTrack.back().push_back(rtrack::wall);

    for (int i = 0; i < originalTrack.size(); i += resolution) {
        lrTrack.push_back(std::vector<char> ());
        lrTrack.back().push_back(rtrack::wall);
        for (int j = 0; j < originalTrack[i].size(); j += resolution) {
            bool start = false;
            bool goal = false;
            bool open = false;
            for (int si = 0;
                    (i + si) < originalTrack.size() && si < resolution;
                    si++) {
                for (int sj = 0;
                        (j + sj) < originalTrack[i].size() && sj < resolution;
                        sj++) {
                    if (originalTrack[i + si][j + sj] != rtrack::wall) {
                        open = true;
                    }
                    if (originalTrack[i + si][j + sj] == rtrack::start) {
                        start = true;
                        starts.insert(std::make_pair(i / resolution + 1,
                                                     j / resolution + 1));
                    }
                    else if (originalTrack[i + si][j + sj] == rtrack::goal) {
                        goal = true;
                        goals.insert(std::make_pair(i / resolution + 1,
                                                    j / resolution + 1));
                    }
                }
            }
            if (start) {
                lrTrack.back().push_back(rtrack::start);
                continue;
            }
            if (goal) {
                lrTrack.back().push_back(rtrack::goal);
                continue;
            }
            if (open)
                lrTrack.back().push_back(rtrack::error);
            else
                lrTrack.back().push_back(rtrack::wall);
        }
        lrTrack.back().push_back(rtrack::wall);
    }

    lrTrack.push_back(std::vector<char> ());
    for (int j = 0; j < originalTrack[0].size(); j += resolution)
        lrTrack.back().push_back(rtrack::wall);
    lrTrack.back().push_back(rtrack::wall);
    lrTrack.back().push_back(rtrack::wall);

    lowResProblem_->track(lrTrack);
    lowResProblem_->starts(starts);
    lowResProblem_->goals(goals);
    lowResProblem_->pSlip(pSlip);
    lowResProblem_->pError(pError);
    lowResProblem_->generateAll();

    mlsolvers::VISolver vi(lowResProblem_, 1000, 0.001);
    vi.solve();

    resolution_ = resolution;
}

RTrackLowResHeuristic::~RTrackLowResHeuristic()
{
    delete lowResProblem_;
}

double RTrackLowResHeuristic::cost(const mlcore::State* s) const
{
    const RacetrackState* rts = static_cast<const RacetrackState*>(s);
    RacetrackState* tmp = new RacetrackState(rts->x() / resolution_ + 1,
                                             rts->y() / resolution_ + 1,
                                             rts->vx() / resolution_,
                                             rts->vy() / resolution_,
                                             lowResProblem_);
    mlcore::StateSet::const_iterator it =
        lowResProblem_->states().find(static_cast<mlcore::State*>(tmp));
    if (it == lowResProblem_->states().end())
        return 0.0;
    delete tmp;
    return .75 * (*it)->cost();
}
