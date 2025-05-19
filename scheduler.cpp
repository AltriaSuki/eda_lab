#include <fstream>
#include <iostream>
#include <unordered_set>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <vector>
#include <sstream>

class schedule{

    enum delay{
        AND=2,
        OR=2,
        NOT=2
    };
    enum colol{
        RED,
        GREEN,
        BLUE,
        YELLOW,
        PINK,
        PURPLE,
        ORANGE,
        CYAN,
        WHITE,
        BLACK
    };

    class node{
    public:
        std::string name;
        std::string type;
        std::unordered_set<node*> successors;
        std::unordered_set<node*> predecessors;
        int start_time;
        int delay;
        bool already_scheduled;
        int depth=0;
        bool inqueue=false;
    };

    std::set<node*> nodes;
public:
    ~schedule(){
        for(auto& n:nodes){
            delete n;
        }
    }

private:
    node* find_node(const std::string& name){
        for(auto& n:nodes){
            if(n->name==name){
                return n;
            }
        }
        return nullptr;
    }
    int print_by_cycle_and_type(){
        std::map<int,std::set<node*>> cycle_map;
        for(auto& n:nodes){
            for(int i=n->start_time;i<n->start_time+n->delay;i++){
                cycle_map[i].insert(n);
            }
        }
        for(auto& pair:cycle_map){
            std::cout<<"Cycle "<<pair.first<<": ";
            for(auto& n:pair.second){
                std::cout<<n->name<<"("<<n->type<<"); ";
            }
            std::cout<<std::endl;
        }
        int max_time=0;
        for(auto& pair:cycle_map){
            if(pair.first>max_time){
                max_time=pair.first;
            }
        }
        return max_time+1;
    }
public:
    void read_blif_file(const std::string& filename){
    
        std::ifstream file(filename);
        if(!file.is_open()){
            std::cerr<<"Error opening file: "<<filename<<std::endl;
            return;
        }
        std::unordered_set<std::string> inputs;
        std::unordered_set<std::string> outputs;
        std::string line;
        while(std::getline(file,line)){
        read_name://读取了.name来避免重新读取行
            if(line.find(".inputs")!=std::string::npos){
                std::istringstream iss(line);
                std::string token;
                while(iss>>token){
                    if(token!=".inputs"){
                        inputs.insert(token);
                    }
                }
            }else if(line.find(".outputs")!=std::string::npos){
                std::istringstream iss(line);
                std::string token;
                while(iss>>token){
                    if(token!=".outputs"){
                        outputs.insert(token);
                    }
                }
            }else if(line.find(".names")!=std::string::npos){
                std::istringstream iss(line);
                std::string token;
                std::vector<std::string> predecessor;
                std::string successor;
                while(iss>>token){
                    if(token!=".names"&&!inputs.count(token)){
                        predecessor.push_back(token);
                        auto it=find_node(token);
                        if(it){
                            continue;
                        }
                        node* new_node=new node();
                        new_node->name=token;
                        new_node->already_scheduled=false;
                        new_node->start_time=0;
                        new_node->delay=0;
                        
                        nodes.insert(new_node);//将节点添加到集合中,如果已经存在则不添加
                    }
                }
                predecessor.pop_back();
                successor=token;
                node* succ_node=find_node(successor);
                //完全不用关心input
                for(auto& pred:predecessor){
                    node* pred_node=find_node(pred);
                    if(pred_node){
                        pred_node->successors.insert(succ_node);
                        succ_node->predecessors.insert(pred_node);
                    }
                }

                while(getline(file,line)){
                    if(line.empty()){
                        break;
                    }else if(line.find(".end")!=std::string::npos||line.find(".name")
                            !=std::string::npos){
                        //整理一下关系，然后跳出循环
                        goto read_name;
                }else{
                    if(line.find("0")!=std::string::npos){
                        succ_node->delay=NOT;
                        succ_node->type="NOT";
                        break;
                    }else if(line.find("-")!=std::string::npos){
                        succ_node->delay=OR;
                        succ_node->type="OR";
                        break;
                    }else if(line.find("1")!=std::string::npos){
                        succ_node->delay=AND;
                        succ_node->type="AND";
                        break;
                    }
                }
                
            }
        }
    }

    file.close();


    std::set<node*> visited;//此处的depth是指从上到下的深度
    while(visited.size()!=nodes.size()){
        for(auto& n:nodes){
            bool all_successors_visited=true;
            for(auto& suc:n->successors){
                if(visited.count(suc)==0){
                    all_successors_visited=false;
                    break;
                }
            }
            if(all_successors_visited){
                if(n->successors.empty()){
                    n->depth=1;
                }else{
                    for(auto& suc:n->successors){
                        n->depth=std::max(n->depth,suc->depth+1);
                    }
            }
            visited.insert(n);
        }
    }
    }

}
struct left_edge_node{
    std::string name;
    int start_time;
    int end_time;
    int color=-1;//表示没有上色
};

struct left_edge_compare{
    bool operator()(const left_edge_node* a,const left_edge_node* b)const{
        if(a->start_time==b->start_time){
            return a->name<b->name;
        }
        return a->start_time<b->start_time;//从小到大排列
    }
};
std::set<left_edge_node*,left_edge_compare> left_edge_set;
void read_left_edge_file(const std::string& filename){
    std::ifstream file(filename);
    if(!file.is_open()){
        std::cerr<<"Error opening file: "<<filename<<std::endl;
        return;
    }
    std::string line;
    while(std::getline(file,line)){
        std::istringstream iss(line);
        std::string token;
        left_edge_node* new_node=new left_edge_node();
        int count=0;
        while(iss>>token){
            count++;
            if(count==1){
                new_node->name=token;
            }else if(count==2){
                new_node->start_time=std::stoi(token);
            }else if(count==3){
                new_node->end_time=std::stoi(token);
            }
        }
        left_edge_set.insert(new_node);
    }
    file.close();
}

void left_edge_schedule(){
    int color=0;
    std::vector<std::set<left_edge_node*,left_edge_compare>> color_set(10);
    int cycle=0;
    while(!left_edge_set.empty()){
        int r=0;
        for(auto it=left_edge_set.begin();it!=left_edge_set.end();){
            auto & n=*it;
            if(n->start_time>=r){
                n->color=color;
                r=n->end_time;
                color_set[color].insert(n);
                it=left_edge_set.erase(it);
            }else{
                it++;
            }
        }
        color++;
    }
    for(int i=0;i<color;i++){
        std::cout<<"Color "<<i<<": "<<std::endl;
        for(auto& n:color_set[i]){
            std::cout<<"----"<<n->name<<"("<<n->start_time<<","<<n->end_time<<")"<<std::endl;
        }
    }
}

int ASAP(){
    std::queue<node*> pq;
    for(auto& n:nodes){
        if(n->predecessors.empty()){
            n->start_time=1;
            pq.push(n);
        }
    }
    while(!pq.empty()){
        node* current=pq.front();
        pq.pop();
        if(current->already_scheduled){
            continue;
        }
        current->already_scheduled=true;
        for(auto& pre:current->predecessors){
            current->start_time=std::max(current->start_time,pre->start_time+pre->delay);
        }
        for(auto& succ:current->successors){
            bool all_scheduled=true;
            for(auto& pre:succ->predecessors){
                if(!pre->already_scheduled)
                    all_scheduled=false;
            }
            if(all_scheduled){
                pq.push(succ);
            }
        }

    }
    int end_time=print_by_cycle_and_type();
    std::cout<<"ASAP scheduling completed."<<std::endl;
    std::ofstream fout("output.txt");  // 打开一个输出文件流，文件名为 output.txt

    if (!fout) {
        std::cerr << "无法打开文件！" << std::endl;
        return 1;
    }
    for(auto& n:nodes){
        fout<<n->name<<" "<<n->start_time<<" "<<n->start_time+n->delay<<std::endl;
    }
    return end_time;
}


void ALAP(int max_time){
    for(auto& n:nodes){
        n->already_scheduled=false;
        n->start_time=max_time-n->delay;
    }
    std::queue<node*> pq;
    for(auto& n:nodes){
        if(n->successors.empty()){
            pq.push(n);
        }
    }
    while(!pq.empty()){
        node* current=pq.front();
        pq.pop();
        if(current->already_scheduled){
            continue;
        }
        current->already_scheduled=true;
        for(auto& succ:current->successors){
            current->start_time=std::min(current->start_time,succ->start_time-current->delay);
        }
        for(auto& pre:current->predecessors){
            bool all_scheduled=true;
            for(auto& succ:pre->successors){
                if(!succ->already_scheduled)
                    all_scheduled=false;
            }
            if(all_scheduled){
                pq.push(pre);
            }

        }
    }
    print_by_cycle_and_type();
    std::cout<<"ALAP scheduling completed."<<std::endl;
}

struct compare_depth{//从大的到小的
    bool operator()(node* a,node*b){
        return a->depth<b->depth;
    }
};

void HU(int how_many){
    std::priority_queue<node*,std::vector<node*>,compare_depth> pq;
    int delay=0;
    for(auto& n:nodes){
        if(n->predecessors.empty()){
            pq.push(n);
        }
    }
    int count=0;
    delay=pq.top()->delay;//所有的delay都一样
    node* last_node=new node();
    last_node->start_time=0;
    last_node->delay=1;//是开始虚拟节点
    node* current=nullptr;
    while(!pq.empty()){
        std::set<node*> visited;
        for(int i=0;i<how_many;i++){
            if(pq.empty())break;
            current=pq.top();
            pq.pop();
            if(current->already_scheduled){
                continue;
            }
            current->start_time=last_node->start_time+last_node->delay;
            current->already_scheduled=true;
            visited.insert(current);
        }
        for(auto& n:visited){
            for(auto& suc:n->successors){
                if(suc->inqueue)continue;
                bool all_scheduled=true;
                for(auto& pre:suc->predecessors){
                    if(!pre->already_scheduled){
                        all_scheduled=false;
                        break;
                    }
                }
                if(all_scheduled){
                    pq.push(suc);
                    suc->inqueue=true;
                }
            }
        }
        last_node=current;
    }
    print_by_cycle_and_type();
    std::cout<<"HU scheduling completed."<<std::endl;
}
};

int main(){
    std::string left_edge_file="left_edge_example.lf";
    std::string asap_output_file="output.txt";
    schedule sch;
    sch.read_blif_file("sample.blif");
    int i=sch.ASAP();
    sch.ALAP(i+1);
    schedule sch2;
    schedule sch3;
    sch2.read_left_edge_file(left_edge_file);
    sch2.left_edge_schedule();
    std::cout<<std::endl;
    sch3.read_left_edge_file(asap_output_file);
    sch3.left_edge_schedule();
    return 0;
}