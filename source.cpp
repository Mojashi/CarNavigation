#include <random>
#include <iostream>
#include <algorithm>
#include <vector>
#include <functional>
#include <array>
#include <fstream>
#include <map>
#include <queue>
#include <cassert>
#include <list>
#include <utility>

using namespace std;

struct Solution;
struct Instance;
struct Graph;
struct Status;

void nextStep(const Instance&, Solution&, Status&, bool addNewCar = false);
void nextStep(const Instance&, Solution&, Status&, int duration);
float simulate(const Instance&);
Solution geneticAlgorithm(const Instance&,const Status&);
Solution commander(const Instance&, const Status&);
Solution genRandSol(const Instance&, const Status&);
void mutation(const Instance&, const Status&,Solution&);
void shorten(const Instance&, const Status&,Solution&);
array<Solution,2> crossOver(const Instance&, const Status&, Solution&, Solution&);

std::random_device seed_gen;
std::mt19937 mt(seed_gen());
uniform_real_distribution<float> rng(0.0, 1.0);

struct Edge{
    int from, to, d;
};
struct Graph{
    vector<vector<Edge>> g;

    int size() const {return g.size();}
};
struct Car{
    int startT, from, to, id;
    float pos; //distance from previous section
};
struct Instance{

    Graph g;
    int N,M, lastStartTime;

    float Vmax,Vmin, Pmut, Psht, Pind0, Pind1;
    int Tstep,Tpred, Npop, Ngen, Tsim, Lsec;
    vector<vector<float>> Kjam;
    vector<Car> cars;

    vector<vector<vector<int>>> edRank;
    vector<vector<int>> dist, edgeIdxTable;

    void read(){
        cin >> Vmax >> Vmin >> Pmut >> Psht >> Pind0 >> Pind1
         >> Tstep >> Tpred >> Npop >> Ngen >> Tsim;
		Lsec = Vmax * Tsim;
		Tstep /= Tsim;
		Tpred /= Tsim;

        cin >> N >> M;
        g.g.resize(N);

        Kjam.resize(N,vector<float>(N));
        edgeIdxTable.resize(N,vector<int>(N, -1));
        for(int i = 0; M > i; i++){
			int f, t, d;
			float jam;

            cin >> f >> t >> d >> jam;
            edgeIdxTable[f][t] = g.g[f].size();
            g.g[f].push_back({f,t,d});
            Kjam[f][t] = jam;
        }

        int carNum;
        cin >> carNum;
        cars.resize(carNum);
		lastStartTime = 0;
        for(int i = 0; carNum > i; i++){
            Car car;
            cin >> car.from >> car.to >> car.startT;
			lastStartTime = max(lastStartTime, car.startT);
            car.id = i;
            car.pos = -1;
            cars[i] = car;
        }

        calcDistance();//init edRank and dist
    }

    int findEdgeIdx(int from, int to)const {
        return edgeIdxTable[from][to];
    }
    int getVertex(int cur, int from, int goal, int rank) const{
        int ud = 0, i;
        for(i = 0; ud <= rank && i < edRank[cur][goal].size(); i++){
            if(edRank[cur][goal][i] != from) 
                ud++;
        }

        return edRank[cur][goal][i-1];
    }

    void calcDistance(){
        dist.resize(g.size());
        edRank.resize(g.size());
        for(int i = 0; g.size() > i; i++){
            dist[i].resize(g.size());
            edRank[i].resize(g.size());
            calcDistance(i);
        }

		for (int i = 0; g.size() > i; i++) {
			for (int j = 0; g.size() > j; j++) {
				vector<pair<int, int>> buf;
				for (auto nex : g.g[i]) {
					buf.push_back(make_pair(nex.d + dist[nex.to][j], nex.to));
				}
				sort(buf.begin(), buf.end());
				for (int k = 0; buf.size() > k; k++) {
					edRank[i][j].push_back(buf[k].second);
				}
			}
		}
    }
    void calcDistance(int from){
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
        
        fill(dist[from].begin(), dist[from].end(), -1);
        pq.push(make_pair(0, from));
        while(pq.size()){
            auto cur = pq.top();
            pq.pop();
            if(dist[from][cur.second] != -1) continue;

            dist[from][cur.second] = cur.first;
            for(auto nex : g.g[cur.second]){
                pq.push(make_pair(nex.d + cur.first , nex.to));
            }
        }
    }

};

struct Solution{
    vector<vector<int>> route;
    bool init = false;
    Solution(){}
    Solution(const Instance& ins){
        route.resize(ins.cars.size());
        init = true;
    }
    void show(FILE* out){
        for(int i = 0; route.size() > i; i++){
            if(route[i].size() == 0) continue;
            fprintf(out, "%d: ", i);
            for(int j = 0; route[i].size() > j; j++)
                fprintf(out, "%d ", route[i][j]);
            fprintf(out, "\n");
        }
    }
};

struct SectionPos{
    int from, to, secNum;
    float dist, goaltime;
    bool goal = false;
};

struct Status{
    int time;
    const Instance *ins;
    vector<vector<vector<list<Car>>>> section;
    map<int, SectionPos> carPos;

    Status(const Instance& ins):ins(&ins){
		time = 0;
        section.resize(ins.g.size());
        for(int i = 0; ins.g.size() > i; i++){
            section[i].resize(ins.g.g[i].size());
            for(int j = 0; ins.g.g[i].size() > j; j++){
                section[i][j].resize((ins.g.g[i][j].d + ins.Lsec-1) / ins.Lsec);
            }
        }
    }

    int rest() const{
        int ret = 0;
        for(auto car : carPos){
            if(!car.second.goal)
                ret++;
        }
        return ret;
    }

    float getDist(Car car) const{
        if(carPos.count(car.id) == 0){
            return ins->dist[car.from][car.to];
        }
        else{
            SectionPos pos = carPos.at(car.id);
			if (pos.goal) return 0;
            return ins->dist[pos.from][car.to] + pos.secNum * ins->Lsec + pos.dist;
        }
    }

    void eraseGoalCar(){
        list<int> keys;
        for(auto& itr : carPos){
            if(itr.second.goal) keys.push_back(itr.first);
        }
        for(auto key : keys)
            carPos.erase(key);
    }
};

Solution genRandSol(const Instance& ins, const Status& stat){
    Solution sol(ins);

    for(int carid = 0; stat.carPos.size() > carid; carid++){
		if (stat.carPos.at(carid).goal) continue;
        Car car = ins.cars[carid];
        int cur = stat.carPos.at(carid).to, bef = -1;
        while(car.to != cur){
            int rank = mt() % ins.g.g[cur].size();
            sol.route[carid].push_back(rank);
            int nex = ins.getVertex(cur, bef, car.to, rank);
			bef = cur;
			cur = nex;
        }
    }
    
    return sol;
}

Solution genShortestPathSol(const Instance& ins, const Status& stat) {
	Solution sol(ins);

	for (int carid = 0; stat.carPos.size() > carid; carid++) {
		if (stat.carPos.at(carid).goal) continue;
		Car car = ins.cars[carid];
		int cur = stat.carPos.at(carid).to, bef = -1;
		while (car.to != cur) {
			int rank = 0;
			sol.route[carid].push_back(rank);
			int nex = ins.getVertex(cur, bef, car.to, rank);
			bef = cur;
			cur = nex;
		}
	}

	return sol;
}


void mutation(const Instance& ins, const Status& stat, Solution& sol){
    for(int carid = 0; stat.carPos.size() > carid; carid++){
        Car car = ins.cars[carid];
        int cur = stat.carPos.at(carid).from, bef = -1;

        for(int i = 0; sol.route[carid].size() > i; i++){
            if(cur == car.to) {
                sol.route[carid].erase(sol.route[carid].begin() + i, sol.route[carid].end());
            }
            if(sol.route[carid].size() == i)
                sol.route[carid].push_back(0);

            //apply upperbound
            sol.route[carid][i] = min(sol.route[carid][i], (int)ins.g.g[cur].size() - 1);
            
            if(rng(mt) < ins.Pmut){
                sol.route[carid][i] = mt() % ins.g.g[cur].size();
            }
            int nex = ins.getVertex(cur, bef, car.to, sol.route[carid][i]);
            bef = cur;
            cur = nex;
        }
    }
}

void shorten(const Instance& ins, const Status& stat, Solution& sol){
    for(int carid = 0; stat.carPos.size() > carid; carid++){
        Car car = ins.cars[carid];
        int cur = stat.carPos.at(carid).from, bef = -1;

        for(int i = 0; sol.route[carid].size() > i; i++){
            if(cur == car.to) {
                sol.route[carid].erase(sol.route[carid].begin() + i, sol.route[carid].end());
            }
            if(sol.route[carid].size() == i)
                sol.route[carid].push_back(0);

            //apply upperbound
            sol.route[carid][i] = min(sol.route[carid][i], (int)ins.g.g[cur].size() - 1);
            
            if(rng(mt) < ins.Psht){
                sol.route[carid][i] = max(0, sol.route[carid][i] - 1);
            }
            int nex = ins.getVertex(cur, bef, car.to, sol.route[carid][i]);
            bef = cur;
            cur = nex;
        }
    }
}

array<Solution,2> crossOver(const Instance& ins, const Status& stat, Solution& sol1, Solution& sol2){
    array<Solution,2> ret = {sol1, sol2};

    for(int carid = 0; ins.cars.size() > carid; carid++){
        if(rng(mt) < 0.5){
            swap(ret[0].route[carid],ret[1].route[carid]);
        }
    }
    
    return ret;
}

void nextStep(const Instance& ins, Solution& sol, Status& stat, int duration) {
	for (int i = 0; duration > i; i++)
		nextStep(ins,sol,stat);
}
void nextStep(const Instance& ins, Solution& sol, Status& stat, bool addNewCar){
    Status ret(ins);
	ret.time = stat.time + 1;
    if(addNewCar){
        for(Car car : ins.cars){
            if(car.startT == stat.time){
                SectionPos newPos;
                newPos.dist = 0;
                newPos.from = car.from;
                newPos.to = ins.getVertex(car.from, -1, car.to, 0);
                newPos.goal = false;
                newPos.goaltime = -1;
                newPos.secNum = 0;
                car.pos = 0;

                if(!newPos.goal)
                    stat.section[newPos.from][ins.findEdgeIdx(newPos.from, newPos.to)][newPos.secNum].push_back(car);
                stat.carPos[car.id] = newPos;
            }
        }
    }

	for (auto section : stat.carPos) {
		if (section.second.goal) continue;
		int from = section.second.from, to = section.second.to, sec = section.second.secNum;
		Car car = ins.cars[section.first];

		float k = 1.0 * (stat.section[from][ins.findEdgeIdx(from, to)][sec].size() - 1) / ins.Lsec;
		float v = max((1 - k / ins.Kjam[from][to]) * ins.Vmax, ins.Vmin);
		float dist = v * ins.Tsim;

		SectionPos newPos(stat.carPos.at(car.id));
		newPos.dist += dist;

		while (newPos.dist > ins.Lsec) {
			if (stat.section[from][ins.findEdgeIdx(from, to)].size() == newPos.secNum + 1) {
				newPos.secNum = 0;
				if (to == car.to) {
					newPos.goal = true;
					newPos.goaltime = stat.time;
					break;
				}
				else {
					int rank = 1;
					if (sol.route[car.id].size()) {
						rank = sol.route[car.id].front();
						sol.route[car.id].erase(sol.route[car.id].begin());
					}

					newPos.from = newPos.to;
					newPos.to = ins.getVertex(newPos.to, newPos.from, car.to, rank);
				}
			}
			else
				newPos.secNum++;
			newPos.dist = newPos.dist - ins.Lsec;
		}

		car.pos = newPos.dist;
		if (!newPos.goal)
			ret.section[newPos.from][ins.findEdgeIdx(newPos.from, newPos.to)][newPos.secNum].push_back(car);
		ret.carPos[car.id] = newPos;
	}
	

    for(auto itr : stat.carPos){
        if(itr.second.goal){
            ret.carPos.insert(itr);
        }
    }

    stat = ret;
}

Status applySol(const Instance& ins, Status curStat, Solution sol, int duration){
	int until = curStat.time + duration;
    for(int t = curStat.time; until > t; t++){
        nextStep(ins, sol, curStat, false);
    }

    return curStat;
}

float geneEval(const Instance& ins, const Status& bef, const Solution& sol){
    float score = 0;
    Status aft = applySol(ins, bef, sol, ins.Tpred);

    for(auto car : bef.carPos){
        float befDist = bef.getDist(ins.cars[car.first]);
        float aftDist = aft.getDist(ins.cars[car.first]);
        auto pos = aft.carPos.at(ins.cars[car.first].id);
        float duration = ins.Tpred;
		if(pos.goal)
			duration = max(0.0f, pos.goaltime - bef.time);
		if(duration > 0)
	        score += (befDist - aftDist) / (ins.Tsim * duration);
    }

    return score / bef.carPos.size();
}

Solution geneticAlgorithm(const Instance& ins, const Status& curStat){
    vector<pair<float, Solution>> gen;
    pair<float, Solution*> ace;
    float Min, sum = 0;

    gen.resize(ins.Npop);
    for(int i = 0; ins.Npop > i; i++){
        Solution sol = genRandSol(ins,curStat);
        gen[i] = {geneEval(ins, curStat, sol), sol};
        if(i > 0){
            if(gen[i].first > ace.first)
                ace = {gen[i].first, &gen[i].second};
            Min = min(Min, gen[i].first-0.1f);
        }
        else{
            ace = {gen[i].first, &gen[i].second};
            Min = gen[i].first-0.1;
        }
        sum += gen[i].first;
    }
    sum -= Min * ins.Npop;

    for(int g = 0; ins.Ngen > g; g++){
        vector<Solution> nextGen;

        while(nextGen.size() < ins.Npop-1){
            Solution *par[2] = {};

            while(par[1] == NULL){
				float roulette = rng(mt), csum = 0;
                for(int i = 0; ins.Npop > i; i++){
					if (sum <= 0) {
						printf("%f %f\n", sum, Min);
						for (int j = 0; ins.Npop > j; j++)
							cout << gen[j].first << endl;
					}
					assert(sum > 0);
                    csum += (gen[i].first - Min) / sum;
                    if(csum > roulette){
                        if(par[0] == NULL) par[0] = &gen[i].second;
                        else par[1] = &gen[i].second;
                        break;
                    }
                }
            }

            auto buf = crossOver(ins, curStat, *par[0], *par[1]);
            nextGen.push_back(buf[0]);
            if(nextGen.size() < ins.Npop-1)
                nextGen.push_back(buf[1]);
        }

        for(auto& sol : nextGen){
            if(rng(mt) < ins.Pind0){
                if(rng(mt) < ins.Pind1)
                    shorten(ins, curStat, sol);
                else
                    mutation(ins, curStat, sol);
            }
        }
        nextGen.push_back(*ace.second);
        
        sum = 0;
        for(int i = 0; ins.Npop > i; i++){
            gen[i] = {geneEval(ins, curStat, nextGen[i]), nextGen[i]};
            if(i > 0){
                if(gen[i].first > ace.first)
                   ace = {gen[i].first, &gen[i].second};
				Min = min(Min, gen[i].first - 0.1f);
			}
			else {
				ace = { gen[i].first, &gen[i].second };
				Min = gen[i].first - 0.1;
			}
            sum += gen[i].first;
        }
        sum -= Min * ins.Npop;
    }   

    return *ace.second;
}

Solution commander(const Instance& ins,const Status& stat){
    return geneticAlgorithm(ins, stat);
}


float simulate(const Instance& ins){
    Status stat(ins);
    Solution schedule(ins);

    for(int t = 0; stat.rest() || t <= ins.lastStartTime; t++){

        if(t %ins.Tstep == 0 && t > 0){
            Status buf(stat);
            Solution buf2(schedule);
			fprintf(stderr, "t=%d remain car=%d Calc routes...\n",t, stat.rest() + ins.cars.size() - stat.carPos.size());
            schedule = commander(ins, buf);
            schedule.show(stderr);
        }
        
        nextStep(ins, schedule, stat, true);
    }

    float ret = 0;

    for(auto car : stat.carPos){
       ret += 1.0*ins.dist[ins.cars[car.first].from][ins.cars[car.first].to] / (car.second.goaltime * ins.Tsim);
    }

    return ret * 3600 / 1000 / ins.cars.size();
}

int main(){
    Instance ins;
    ins.read();

    float avg = simulate(ins);

    cout << avg << endl;

    return 0;
}
