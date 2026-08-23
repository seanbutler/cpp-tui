#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>
#include <functional>

namespace tui::widgets {

// A single node in the tree.
struct TreeNode {
    std::string           label;
    bool                  expanded = false;
    std::vector<TreeNode*> children;

    // Set by Tree when building the flat row list — do not set manually.
    int level = 0;
};

// Tree renders a collapsible/expandable tree of nodes.
// Navigate with scrollUp/scrollDown; toggle with toggleExpand.
//
//  ┌ File Tree ──────────────────────────────────┐
//  │ − src                                       │
//  │   − widgets                                 │
//  │       paragraph.cpp                         │
//  │       list.cpp                              │
//  │   buffer.cpp                                │
//  │ + include                                   │
//  └─────────────────────────────────────────────┘
class Tree : public Block {
public:
    Style textStyle        = Theme.tree.text;
    Style selectedRowStyle = Theme.tree.text;

    Tree();

    // Set the top-level nodes.  Rebuilds the visible row list.
    void setNodes(std::vector<TreeNode*> nodes);

    // Navigation.
    void scrollUp()       { scrollAmount(-1); }
    void scrollDown()     { scrollAmount(1); }
    void scrollTop()      { selectedRow_ = 0; }
    void scrollBottom()   { selectedRow_ = static_cast<int>(rows_.size()) - 1; }
    void scrollAmount(int delta);

    // Expand / collapse the selected node.
    void expand();
    void collapse();
    void toggleExpand();
    void expandAll();
    void collapseAll();

    TreeNode* selectedNode();

    void draw(Buffer& buf) override;

private:
    std::vector<TreeNode*> roots_;  // top-level nodes
    std::vector<TreeNode*> rows_;   // visible (flattened) nodes
    int selectedRow_ = 0;
    int topRow_      = 0;

    void rebuildRows();
    void flattenNode(TreeNode* node, int level);

    void walkAll(const std::function<void(TreeNode*)>& fn);
};

} // namespace tui::widgets
