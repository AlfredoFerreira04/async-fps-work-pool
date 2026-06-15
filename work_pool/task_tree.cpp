#include <concepts>
#include <type_traits>
#include <memory>
#include <stdexcept>

// Concept: requires T to have getPriority() and return type is totally ordered
template<typename T>
concept TreeNodeValueType = requires(T t) {
    { t.getPriority() };
} && std::totally_ordered<std::invoke_result_t<decltype(&T::getPriority), T>>;

enum Color {
    RED,
    BLACK
};

template<TreeNodeValueType T>
class TaskTree {
    struct TreeNode {
        T value;

        Color color;
        std::shared_ptr<TreeNode> father;
        std::shared_ptr<TreeNode> left;
        std::shared_ptr<TreeNode> right;

        // Root constructor
        TreeNode(const T& value)
            : value(value),
              color(BLACK),
              father(nullptr),
              left(nullptr),
              right(nullptr) {}

        // Non-root constructor
        TreeNode(const T& value, std::shared_ptr<TreeNode> father)
            : value(value),
              color(RED),
              father(father),
              left(nullptr),
              right(nullptr) {}
    };

    using NodePtr = std::shared_ptr<TreeNode>;

    NodePtr rootNode = nullptr;

    // ---------------- HELPERS ----------------

    NodePtr minimum(NodePtr x) {
        while (x->left)
            x = x->left;
        return x;
    }

    void transplant(NodePtr u, NodePtr v) {
        if (!u->father)
            rootNode = v;
        else if (u == u->father->left)
            u->father->left = v;
        else
            u->father->right = v;

        if (v)
            v->father = u->father;
    }

    // ---------------- ROTATIONS ----------------

    void rotate_left(NodePtr x) {
        NodePtr y = x->right;

        x->right = y->left;
        if (y->left)
            y->left->father = x;

        y->father = x->father;

        if (!x->father)
            rootNode = y;
        else if (x == x->father->left)
            x->father->left = y;
        else
            x->father->right = y;

        y->left = x;
        x->father = y;
    }

    void rotate_right(NodePtr y) {
        NodePtr x = y->left;

        y->left = x->right;
        if (x->right)
            x->right->father = y;

        x->father = y->father;

        if (!y->father)
            rootNode = x;
        else if (y == y->father->left)
            y->father->left = x;
        else
            y->father->right = x;

        x->right = y;
        y->father = x;
    }

    // ---------------- FIX INSERT ----------------

    void fix_insert(NodePtr z) {
        while (z->father && z->father->color == RED) {

            NodePtr p = z->father;
            NodePtr g = p->father;
            if (!g) break;

            if (p == g->left) {
                NodePtr u = g->right;

                // Case 1: uncle red
                if (u && u->color == RED) {
                    p->color = BLACK;
                    u->color = BLACK;
                    g->color = RED;
                    z = g;
                }
                else {
                    // Case 2: LR
                    if (z == p->right) {
                        z = p;
                        rotate_left(z);
                        p = z->father;
                        g = p->father;
                    }

                    // Case 3: LL
                    p->color = BLACK;
                    g->color = RED;
                    rotate_right(g);
                }
            }
            else {
                NodePtr u = g->left;

                // Case 1: uncle red
                if (u && u->color == RED) {
                    p->color = BLACK;
                    u->color = BLACK;
                    g->color = RED;
                    z = g;
                }
                else {
                    // Case 2: RL
                    if (z == p->left) {
                        z = p;
                        rotate_right(z);
                        p = z->father;
                        g = p->father;
                    }

                    // Case 3: RR
                    p->color = BLACK;
                    g->color = RED;
                    rotate_left(g);
                }
            }
        }

        rootNode->color = BLACK;
    }

    // ---------------- INSERT ----------------

    NodePtr push(NodePtr& curr, const T& val, NodePtr parent = nullptr) {
        if (!curr) {
            curr = std::make_shared<TreeNode>(val, parent);
            return curr;
        }

        if (val.getPriority() < curr->value.getPriority())
            return push(curr->left, val, curr);

        else if (val.getPriority() > curr->value.getPriority())
            return push(curr->right, val, curr);

        else
            return push(curr->right, val, curr);
    }

    // ---------------- FIX DELETE ----------------

    void fix_delete(NodePtr x) {
        while (x != rootNode && (!x || x->color == BLACK)) {

            NodePtr p = x ? x->father : nullptr;
            if (!p) break;

            if (x == p->left) {
                NodePtr s = p->right;

                // Case 1: sibling red
                if (s && s->color == RED) {
                    s->color = BLACK;
                    p->color = RED;
                    rotate_left(p);
                    s = p->right;
                }

                // Case 2: sibling black, both children black
                if ((!s->left || s->left->color == BLACK) &&
                    (!s->right || s->right->color == BLACK)) {

                    if (s) s->color = RED;
                    x = p;
                }
                else {
                    // Case 3: far child black, near red
                    if (!s->right || s->right->color == BLACK) {
                        if (s->left) s->left->color = BLACK;
                        s->color = RED;
                        rotate_right(s);
                        s = p->right;
                    }

                    // Case 4: far child red
                    s->color = p->color;
                    p->color = BLACK;
                    if (s->right) s->right->color = BLACK;
                    rotate_left(p);

                    x = rootNode;
                }
            }
            else {
                NodePtr s = p->left;

                // Case 1
                if (s && s->color == RED) {
                    s->color = BLACK;
                    p->color = RED;
                    rotate_right(p);
                    s = p->left;
                }

                // Case 2
                if ((!s->left || s->left->color == BLACK) &&
                    (!s->right || s->right->color == BLACK)) {

                    if (s) s->color = RED;
                    x = p;
                }
                else {
                    // Case 3
                    if (!s->left || s->left->color == BLACK) {
                        if (s->right) s->right->color = BLACK;
                        s->color = RED;
                        rotate_left(s);
                        s = p->left;
                    }

                    // Case 4
                    s->color = p->color;
                    p->color = BLACK;
                    if (s->left) s->left->color = BLACK;
                    rotate_right(p);

                    x = rootNode;
                }
            }
        }

        if (x)
            x->color = BLACK;
    }

    NodePtr get_highest(NodePtr curr) {
        while (curr && curr->right)
            curr = curr->right;
        return curr;
    }

    T get_and_delete_right_most() {
        NodePtr highest = get_highest(rootNode);
        T returnVal = highest->value;

        erase(highest->value);

        return returnVal;
    }

public:
    bool empty() const {
        return !rootNode;
    }

    void put(const T& value) {
        NodePtr inserted = push(rootNode, value);
        fix_insert(inserted);
    }

    void erase(const T& val) {
        NodePtr z = rootNode;

        // find node
        while (z) {
            if (val.getPriority() < z->value.getPriority())
                z = z->left;
            else if (val.getPriority() > z->value.getPriority())
                z = z->right;
            else
                break;
        }

        if (!z) return;

        NodePtr y = z;
        Color y_original_color = y->color;
        NodePtr x;

        if (!z->left) {
            x = z->right;
            transplant(z, z->right);
        }
        else if (!z->right) {
            x = z->left;
            transplant(z, z->left);
        }
        else {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;

            if (y->father == z) {
                if (x) x->father = y;
            }
            else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->father = y;
            }

            transplant(z, y);
            y->left = z->left;
            y->left->father = y;
            y->color = z->color;
        }

        if (y_original_color == BLACK)
            fix_delete(x);
    }

    T pop_right_most() {
        if (!rootNode)
            throw std::runtime_error("TaskTree is empty");

        return get_and_delete_right_most();
    }
};