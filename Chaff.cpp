#include<iostream>
#include<cassert>
#include<map>
#include<vector>
#include<fstream>
#include<sstream>
#include<set>
using namespace std;
typedef vector<int>  clause;

int curr_level;
vector<int> value;
vector<clause> clauses;
vector<int> assign_level;
vector<pair<int,int>> assign_stack;
vector<int> antecedent;
int conflict_clause;

bool BCP()
{
    while(1)
    {
        int i=0, flag=0;
        for(auto &c: clauses)
        {
            bool cvalue = 0;
            int not_assigned=0, lit;
            for (auto &l : c)
            {
                if (value[abs(l)] != -1)
                {
                    cvalue |= (l > 0) ? value[abs(l)] : !value[abs(l)];
                    if (cvalue)
                        break;
                }
                else 
                {
                    not_assigned++;
                    lit = l;
                }
            }
            if (cvalue)
            {
                i++;
                continue;
            }
            if(not_assigned == 0)
            {
                conflict_clause = i;
                return 1;
            }
            else if(not_assigned == 1)
            {
                flag = 1;
                assert(value[abs(lit)]==-1);
                value[abs(lit)] = (lit > 0) ? 1 : 0;
                cout << "propagate to " << abs(lit) << " = " << value[abs(lit)] <<" at level "<<curr_level<<"\n";
                assign_level[abs(lit)] = curr_level;
                antecedent[abs(lit)] = i;
                assign_stack.push_back({lit, curr_level});
                break;
            }
            i++;
        }
        if(!flag)
            break;
    }
    return 0;
}

clause resolve(clause &cl1, clause &cl2, int var)
{
    clause resolvent;
    set<int> lit;
    for (auto l : cl1)
        if (abs(l) != var)
            lit.insert(l);
    for (auto l : cl2)
        if (abs(l) != var)
            lit.insert(l);
    for(auto l: lit)
        resolvent.push_back(l);
    return resolvent;
}

int analyze_conflict()
{
    if(curr_level == 0)
        return -1;
    clause cl = clauses[conflict_clause];
    while(1)
    {
        assert(cl.size());
        //stopping criterion
        int cnt=0;
        cout<<curr_level<<"- ";
        for(auto &l: cl)
        {
            if(assign_level[abs(l)] == curr_level)
                cnt++;
            cout<<l<<":"<<assign_level[abs(l)]<<" ";
        }
        if(cnt == 1)
            break;
        assert(cnt);

        //choose literal
        int lit;
        while(!assign_stack.empty())
        {
            int level = assign_stack.back().second;
            int var = abs(assign_stack.back().first);

            if(level == curr_level)
            {
                bool found = 0;
                for (auto &l : cl)
                {
                    if (abs(l) == var)
                    {
                        found = 1;
                        break;
                    }
                }
                if(found)
                    break;
            }
            cout<<"pop "<<var<<"\n";
            value[var] = -1;
            assign_level[var] = -1;
            antecedent[var] = -1;
            assign_stack.pop_back();
        }
        assert(assign_stack.size());
        lit = assign_stack.back().first;
        assign_stack.pop_back();
        cout<<"literal: "<<lit<<"\n";        
        int ante = antecedent[abs(lit)];
        assert(ante!=-1);
        cl = resolve(cl, clauses[ante], abs(lit));
        value[abs(lit)] = -1;
        assign_level[abs(lit)] = -1;
        antecedent[abs(lit)] = -1;
    }
    clauses.push_back(cl);
    int back_level = 0;
    for (auto &l: cl)
    {
        if(assign_level[abs(l)] != curr_level)
        {
            back_level = max(back_level, assign_level[abs(l)]);
        }
    }
    cout<<"backtrack to "<<back_level<<"!\n";
    return back_level;
}

bool decide()
{
    for(int var=1; var<value.size(); ++var)
    {
        if(value[var]==-1)
        {
            cout<<"decide "<<var<<" = 1 at level "<<curr_level<<"\n";
            value[var] = 1;
            assign_level[var] = curr_level;
            antecedent[var] = -1;
            assign_stack.push_back({var, curr_level});
            return 1;
        }
    }
    return 0;
}

void backtrack()
{
    cout<<"stack before backtrack: ";
    for(auto x: assign_stack)
        cout<<x.second<<" ";
    cout<<"\n";
    while(assign_stack.size() && assign_stack.back().second > curr_level)
    {
        int var = abs(assign_stack.back().first);
        value[var] = -1;
        assign_level[var] = -1;
        antecedent[var] = -1;
        assign_stack.pop_back();
    }
    
    cout<<"stack after backtrack: ";
    for(auto x: assign_stack)
        cout<<x.second<<" ";
    cout<<"\n";
    
    // cout<<"variable dlevel: ";
    // for(int var=1; var<value.size(); ++var)
    // {
    //     cout<<var<<":"<<assign_level[var]<<"\n";
    // }
    // cout<<"\n";
}

bool Chaff()
{
    if(BCP())
        return 0;
    while(1)
    {
        curr_level ++;
        if(!decide()) return 1;
        while(BCP())
        {
            int blevel = analyze_conflict();
            if(blevel<0) return 0;
            curr_level = blevel;
            backtrack();
        }
    }
    return 1;
}

void read_CNF(string filename)
{
    string line;
    ifstream inp(filename, ios::in);
    if (!inp)
    {
        cerr << "Can't open input file" << endl;
        exit(1);
    }
    int var_num, cl_num, cl_idx = 0;
    while (getline(inp, line))
    {
        cout.flush();
        if (line[0] == 'c')
            continue;
        if (line[0] == 'p')
        {
            char ch;
            string mode;
            istringstream ss(line);
            ss >> ch >> mode;
            assert(mode == "CNF" || mode == "cnf");
            ss >> var_num >> cl_num;
            clauses = vector<clause>(cl_num);
            value = vector<int>(var_num + 1, -1);
            assign_level = vector<int>(var_num + 1, -1);
            antecedent = vector<int>(var_num + 1, -1);
        }
        else
        {
            int i = 0, n = line.length();
            while (1)
            {
                string word;
                while (i < n && (line[i] == ' ' || line[i] == '\t'))
                    i++;
                while (i < n && (line[i] != ' ' && line[i] != '\t'))
                {
                    word += line[i];
                    i++;
                }
                int var_idx = stoi(word);
                if (var_idx == 0 || i == n)
                    break;
                clauses[cl_idx].push_back(var_idx);
            }
            cl_idx++;
        }
    }
}

int32_t main(int32_t argc, char **argv)
{
    if (argc < 2)
    {
        cerr << "Enter filename as command line argument!\n";
        exit(4);
    }
    read_CNF(argv[1]);
    bool issatisfiable = Chaff();
    if (issatisfiable)
    {
        cout << "SATISFIABLE!\n";
    }
    else
    {
        cout << "UNSATISFIABLE!\n";
    }
}
