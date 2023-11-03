#include "InternalNode.hpp"
#include "LeafNode.hpp"
#include "RecordPtr.hpp"
//creates internal node pointed to by tree_ptr or creates a new one
InternalNode::InternalNode(const TreePtr &tree_ptr) : TreeNode(INTERNAL, tree_ptr) {
    this->keys.clear();
    this->tree_pointers.clear();
    if (!is_null(tree_ptr))
        this->load();
}

//max element from tree rooted at this node
Key InternalNode::max() {
    Key max_key = DELETE_MARKER;
    TreeNode* last_tree_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    max_key = last_tree_node->max();
    delete last_tree_node;
    return max_key;
}

//if internal node contains a single child, it is returned
TreePtr InternalNode::single_child_ptr() {
    if (this->size == 1)
        return this->tree_pointers[0];
    return NULL_PTR;
}

//inserts <key, record_ptr> into subtree rooted at this node.
//returns pointer to split node if exists
//TODO: InternalNode::insert_key to be implemented
TreePtr InternalNode::insert_key(const Key &key, const RecordPtr &record_ptr) {
    TreePtr new_tree_ptr = NULL_PTR;
//    cout << "InternalNode::insert_key not implemented" << endl;

    // finding appropriate child pointer to insert at
    TreePtr child_tree_ptr = NULL_PTR;
    int i=this->keys.size();
    if(key > this->keys.back()) child_tree_ptr = this->tree_pointers.back();
    else { for(i=0; i<this->keys.size(); i++) { if(key <= this->keys[i]) { child_tree_ptr = this->tree_pointers[i]; break;} } }

    // sending the key to child node to insert
    TreeNode* child_node = TreeNode::tree_node_factory(child_tree_ptr);
    auto split_child_tree_ptr = child_node->insert_key(key, record_ptr);

    //If the child has split, adding to current node
    if(!is_null(split_child_tree_ptr))
    {
        // adding split child to current node
        int new_key = child_node->max();
        this->keys.push_back(new_key);
        for(int j=this->keys.size()-1; j>i; j--) this->keys[j] = this->keys[j-1];
        this->keys[i]=new_key;
        this->tree_pointers.push_back(split_child_tree_ptr);
        for(int j=this->tree_pointers.size()-1; j>i+1; j--) this->tree_pointers[j] = this->tree_pointers[j-1];
        this->tree_pointers[i+1]=split_child_tree_ptr;
        this->size++;

        // checking if we need to split the current node
        if(this->size > FANOUT)
        {
            //making new internal node part and deleting elements from current leaf
            auto new_internal_node = new InternalNode();
            int it = ceil((float)this->size/2);
            for(int j=it  ; j<this->size       ; j++) new_internal_node->tree_pointers.push_back(this->tree_pointers[j]);
            for(int j=it  ; j<this->size       ; j++) this->tree_pointers.pop_back();
            for(int j=it  ; j<this->keys.size(); j++) new_internal_node->keys.push_back(this->keys[j]);
            for(int j=it-1; j<this->keys.size(); j++) this->keys.pop_back();

            // rearranging pointer, modifying size and dumping
            new_tree_ptr = new_internal_node->tree_ptr;
            this->size=this->tree_pointers.size();
            new_internal_node->size=new_internal_node->tree_pointers.size();
            new_internal_node->dump();
            delete new_internal_node;
        }
    }

    this->dump();
    return new_tree_ptr;
}

//deletes key from subtree rooted at this if exists
//TODO: InternalNode::delete_key to be implemented
void InternalNode::delete_key(const Key &key) {
    TreePtr new_tree_ptr = NULL_PTR;
//    cout << "InternalNode::delete_key not implemented" << endl;
    // finding appropriate child pointer to delete at
    TreePtr child_tree_ptr = NULL_PTR;
    int i=this->keys.size();
    if(key > this->keys.back()) child_tree_ptr = this->tree_pointers.back();
    else { for(i=0; i<this->keys.size(); i++) { if(key <= this->keys[i]) { child_tree_ptr = this->tree_pointers[i]; break;} } }

    // sending the key to child node to delete
    TreeNode* child_node = TreeNode::tree_node_factory(child_tree_ptr);
    child_node->delete_key(key);

    //checking if child node is less than half full to redistribute or merge
    if(child_node->size < ceil((float)FANOUT/2))
    {
        if(i>0) //operating with left neighbour
        {
            TreeNode* neighbour_child_node = TreeNode::tree_node_factory(this->tree_pointers[i-1]);
            if(child_node->size + neighbour_child_node->size > FANOUT)
            {
                //perform redistribute
                if(child_node->node_type==LEAF)
                {
                    // l=left node, r=right node
                    LeafNode* l = new LeafNode(this->tree_pointers[i-1]);
                    LeafNode* r = new LeafNode(this->tree_pointers[i]);
                    l->size--; r->size++;
                    r->data_pointers[l->max()] = l->data_pointers[l->max()];
                    l->data_pointers.erase(l->max());
                    this->keys[i-1] = l->max();
                    l->dump(); l->dump();
                }
                else // redistribute on internal node
                {
                    // l=left node, r=right node
                    InternalNode* l = new InternalNode(this->tree_pointers[i-1]);
                    InternalNode* r = new InternalNode(this->tree_pointers[i]);
                    l->size--; r->size++;

                    // moving values to start of the lists in the right child
                    r->tree_pointers.push_back(l->tree_pointers.back());
                    for(int j=r->size-1;j>0;j--) r->tree_pointers[j] = r->tree_pointers[j-1];
                    r->tree_pointers[0] = l->tree_pointers.back();
                    l->tree_pointers.pop_back(); l->keys.pop_back();
                    TreeNode* temp_ptr = tree_node_factory(l->tree_pointers[0]);
                    r->keys.push_back(temp_ptr->max());
                    for(int j=r->size-2;j>0;j--) r->keys[j] = r->keys[j-1];
                    r->keys[0] = temp_ptr->max();
                    this->keys[i-1] = l->max();
                    l->dump(); r->dump();
                }
            }
            else
            {
                //perform merge
                if(child_node->node_type==LEAF)
                {
                    LeafNode * l = new LeafNode(this->tree_pointers[i-1]);
                    LeafNode * r = new LeafNode(this->tree_pointers[i]);
                    for(auto it = r->data_pointers.begin(); it!= r->data_pointers.end();it++) l->data_pointers[it->first] = it->second;
                    l->next_leaf_ptr = r->next_leaf_ptr;
                    l->size = l->data_pointers.size(); r->size = 0;
                    l->dump(); r->dump();
                }
                else //perform merge on internal node
                {
                    InternalNode * l = new InternalNode(this->tree_pointers[i-1]);
                    InternalNode * r = new InternalNode(this->tree_pointers[i]);
                    r->keys.push_back(r->max());
                    for(int j=0;j<r->size;j++)
                    {
                        l->tree_pointers.push_back(r->tree_pointers[j]);
                        if(j!=r->size-1) l->keys.push_back(r->keys[j]);
                    }
                    l->size=l->tree_pointers.size(); r->size = 0;
                    l->dump(); r->dump();
                }
                // removing element from current node
                for(int j=i; j<this->size-1; j++) this->tree_pointers[j] = this->tree_pointers[j+1];
                for(int j=i-1; j<this->size-2; j++) this->keys[j] = this->keys[j+1];
                this->tree_pointers.pop_back(); this->keys.pop_back(); this->size--;
            }
        }
        else //operating on right neighbour
        {
            TreeNode* neighbour_child_node = TreeNode::tree_node_factory(this->tree_pointers[i+1]);
            if(child_node->size + neighbour_child_node->size > FANOUT)
            {
                //perform redistribute
                if(child_node->node_type==LEAF)
                {
                    // l=left node(y), r=right node(x)
                    LeafNode* l = new LeafNode(this->tree_pointers[i]);
                    LeafNode* r = new LeafNode(this->tree_pointers[i+1]);
                    l->size++; r->size--;
                    l->data_pointers[r->data_pointers.begin()->first] = r->data_pointers[r->data_pointers.begin()->first];
                    r->data_pointers.erase(r->data_pointers.begin()->first);
                    this->keys[i] = l->max();
                    l->dump(); r->dump();
                }
                else //perform redistribute on internal node
                {
                    // l=left node, r=right node
                    InternalNode* l = new InternalNode(this->tree_pointers[i]);
                    InternalNode* r = new InternalNode(this->tree_pointers[i+1]);
                    l->size++; r->size--;

                    // moving values to end of the lists in the left child
                    l->tree_pointers.push_back(r->tree_pointers.front());
                    r->tree_pointers.erase(r->tree_pointers.begin());
                    r->keys.erase(r->keys.begin());

                    // adding key to left child
                    TreeNode * temp_ptr = tree_node_factory(l->tree_pointers[l->size-2]);
                    l->keys.push_back(temp_ptr->max());
                    this->keys[i] = l->max();
                    l->dump(); r->dump();
                }
            }
            else
            {
                //perform merge
                if(child_node->node_type==LEAF)
                {
                    LeafNode * l = new LeafNode(this->tree_pointers[i]);
                    LeafNode * r = new LeafNode(this->tree_pointers[i+1]);
                    for(auto it = r->data_pointers.begin(); it!= r->data_pointers.end();it++) l->data_pointers[it->first] = it->second;
                    l->next_leaf_ptr = r->next_leaf_ptr;
                    l->size = l->data_pointers.size(); r->size = 0;
                    l->dump(); r->dump();
                }
                else //perform merge on internal node
                {
                    InternalNode * l = new InternalNode(this->tree_pointers[i]);
                    InternalNode * r = new InternalNode(this->tree_pointers[i+1]);
                    r->keys.push_back(r->max());
                    for(int j=0;j<r->size;j++)
                    {
                        l->tree_pointers.push_back(r->tree_pointers[j]);
                        if(j!=r->size-1) l->keys.push_back(r->keys[j]);
                    }
                    l->size=l->tree_pointers.size(); r->size = 0;
                    l->dump(); r->dump();
                }
                // removing element from current node
                for(int j=i+1; j<this->size-1; j++) this->tree_pointers[j] = this->tree_pointers[j+1];
                for(int j=i; j<this->size-2; j++) this->keys[j] = this->keys[j+1];
                this->tree_pointers.pop_back(); this->keys.pop_back(); this->size--;
            }
        }
    }
    this->dump();
}

//runs range query on subtree rooted at this node
void InternalNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for (int i = 0; i < this->size - 1; i++) {
        if (min_key <= this->keys[i]) {
            auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
            child_node->range(os, min_key, max_key);
            delete child_node;
            return;
        }
    }
    auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    child_node->range(os, min_key, max_key);
    delete child_node;
}

//exports node - used for grading
void InternalNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for (int i = 0; i < this->size - 1; i++)
        os << this->keys[i] << " ";
    os << endl;
    for (int i = 0; i < this->size; i++) {
        auto child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        child_node->export_node(os);
        delete child_node;
    }
}

//writes subtree rooted at this node as a mermaid chart
void InternalNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    chart_node += "]";
    os << chart_node << endl;

    for (int i = 0; i < this->size; i++) {
        auto tree_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        tree_node->chart(os);
        delete tree_node;
        string link = this->tree_ptr + "-->|";

        if (i == 0)
            link += "x <= " + to_string(this->keys[i]);
        else if (i == this->size - 1) {
            link += to_string(this->keys[i - 1]) + " < x";
        } else {
            link += to_string(this->keys[i - 1]) + " < x <= " + to_string(this->keys[i]);
        }
        link += "|" + this->tree_pointers[i];
        os << link << endl;
    }
}

ostream& InternalNode::write(ostream &os) const {
    TreeNode::write(os);
    for (int i = 0; i < this->size - 1; i++) {
        if (&os == &cout)
            os << "\nP" << i + 1 << ": ";
        os << this->tree_pointers[i] << " ";
        if (&os == &cout)
            os << "\nK" << i + 1 << ": ";
        os << this->keys[i] << " ";
    }
    if (&os == &cout)
        os << "\nP" << this->size << ": ";
    os << this->tree_pointers[this->size - 1];
    return os;
}

istream& InternalNode::read(istream& is) {
    TreeNode::read(is);
    this->keys.assign(this->size - 1, DELETE_MARKER);
    this->tree_pointers.assign(this->size, NULL_PTR);
    for (int i = 0; i < this->size - 1; i++) {
        if (&is == &cin)
            cout << "P" << i + 1 << ": ";
        is >> this->tree_pointers[i];
        if (&is == &cin)
            cout << "K" << i + 1 << ": ";
        is >> this->keys[i];
    }
    if (&is == &cin)
        cout << "P" << this->size;
    is >> this->tree_pointers[this->size - 1];
    return is;
}
