#include<iostream>
#include<map>
#include<vector>
#include<cassert>

using namespace std;
#define int long long 
typedef vector<int> clause;
struct order
{
	int order_in_level, dlevel;
};
bool BCP(int &conflict_clause, int dlevel, vector<clause> &clauses, map<int,pair<order,bool>> &assignment, map<int,int> &antecedent, map<int,int> &latest)
{
	latest[dlevel] = 0;
	while(1)
	{
		bool flag = 0;
		int i=0;
		for(auto &c: clauses)
		{
			int not_assigned = 0;
			int lit;
			bool value = 0;
			for(auto &l: c)
			{
				if(assignment.find(abs(l)) == assignment.end())
				{
					not_assigned ++;
					lit = l;	
				}
				else
					value |= (l > 0) ? assignment[abs(l)].second : !assignment[abs(l)].second;
			}
			if(value)
			{
				i++;
				continue;
			}
			if(not_assigned == 0 && value == 0)
			{
				//conflict!
				conflict_clause = i;
				return 1;
			}
			if(not_assigned == 1)
			{
				flag = 1;
				assignment[abs(lit)] = {{.order_in_level = latest[dlevel]++, .dlevel = dlevel}, (lit>0)?1:0};
				antecedent[abs(lit)] = i;
			}
			i++;
		}
		if(!flag) break;
	}
	return 0;
}
clause resolve(clause &cl1, clause &cl2, int var)
{
	clause resolvent;
	for(auto l: cl1)
		if(abs(l)!=var)
			cl1.push_back(l);
	for(auto l: cl2)
		if(abs(l)!=var)
			cl2.push_back(l);
	return resolvent;
}
int analyze_conflict(int conflict_clause, int dlevel, vector<clause> &clauses, map<int, pair<order, bool>> &assignment, map<int, int> &antecedent, map<int, int> &latest)
{
	if(dlevel == 0)
		return -1;
	clause cl = clauses[conflict_clause]; 
	while(1)
	{
		//stopping criterion
		int cnt=0;
		for(auto &l: cl)
		{
			if(assignment[abs(l)].first.dlevel == dlevel)
				cnt++;
		}
		if(cnt==1) break;

		//choose literal
		int lit;
		int max_order=-1;
		for(auto &l: cl)
		{
			if(max_order<assignment[abs(l)].first.order_in_level)
			{
				max_order = assignment[abs(l)].first.order_in_level;
				lit = l;
			}
		}
		assert(max_order!=-1);
		int ante = antecedent[abs(lit)];
		cl = resolve(cl, clauses[ante], abs(lit));
	}
	clauses.push_back(cl);
	int back_level = 0;
	for(auto &l: cl)
	{
		if(assignment[abs(l)].first.dlevel != dlevel)
		{
			back_level = max(back_level, assignment[abs(l)].first.dlevel);
		}
		else 
		{
			assignment[abs(l)] = {{.order_in_level = latest[back_level]++, .dlevel = back_level,}, !assignment[abs(l)].second};
	}
	}
	return back_level;
}	
			
bool decide(map<int, pair<order, bool>> &assignment, int dlevel, vector<clause>&clauses, map<int, int> &latest)
{
	for(auto &c: clauses)
	{
		for(auto l: c)
		{
			if(assignment.find(abs(l)) == assignment.end())
			{
				assignment[abs(l)] = {{.order_in_level = latest[dlevel]++, .dlevel = dlevel}, 1};
				return 1;
			}
		}
	}
	return 0;
}	

void back_track(int dlevel, map<int, pair<order, bool>> &assignment)
{
	vector<int>rem;
	for(auto p: assignment)
	{
		if(p.second.first.dlevel>dlevel)
		{
			rem.push_back(p.first);
		}
	}
	for(auto x: rem)
		assignment.erase(x);
}	

bool Chaff(vector<clause> &clauses, map<int, pair<order, bool>> &assignment)
{
	map<int, int> antecedent;
	int dlevel=0;
	int conflict_clause;
	map<int,int>latest;
	if(BCP(conflict_clause, dlevel, clauses, assignment, antecedent, latest))
		return 0;
	while(1)
	{
		if(!decide(assignment, dlevel, clauses, latest)) return 1;

		while (BCP(conflict_clause, dlevel, clauses, assignment, antecedent, latest))
		{
			int blevel = analyze_conflict(conflict_clause, dlevel, clauses, assignment, antecedent, latest);
			if(blevel<0)
				return 1;
			else back_track(dlevel, assignment);
			dlevel = blevel;
		}
		dlevel++;
	}
	return 1;
}
vector<clause> read_CNF()
{
	int n;
	cin>>n;
	vector<clause> clauses(n);
	for(int i=0; i<n; ++i)
	{
		int m;
		cin>>m;
		for(int j=0; j<m; ++j)
		{
			int lit;
			cin>>lit;
			clauses[i].push_back(lit);
		}
	}
	return clauses;
}
int32_t main()
{
	vector<clause>clauses = read_CNF();
	map<int, pair<order, bool>> assignment;
	bool issatisfiable = Chaff(clauses, assignment);
	if(issatisfiable)
	{
		cout << "SATISFIABLE!\n";
		for (auto p : assignment)
		{
			cout << p.first << " " << p.second.second << '\n';
		}
	}
	else 
	{
		cout<<"UNSATISFIABLE!\n";
	}
}
