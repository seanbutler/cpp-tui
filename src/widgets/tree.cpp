#include "tui/widgets/tree.hpp"
#include "tui/symbols.hpp"
#include <algorithm>

namespace tui::widgets {

static constexpr int INDENT = 2;

Tree::Tree() {
    textStyle        = Theme.tree.text;
    selectedRowStyle = Theme.tree.text;
}

void Tree::setNodes(std::vector<TreeNode*> nodes) {
    roots_ = std::move(nodes);
    rebuildRows();
}

void Tree::rebuildRows() {
    rows_.clear();
    for (TreeNode* root : roots_)
        flattenNode(root, 0);
}

void Tree::flattenNode(TreeNode* node, int level) {
    node->level = level;
    rows_.push_back(node);
    if (node->expanded) {
        for (TreeNode* child : node->children)
            flattenNode(child, level + 1);
    }
}

void Tree::scrollAmount(int delta) {
    int n = static_cast<int>(rows_.size());
    selectedRow_ = std::clamp(selectedRow_ + delta, 0, std::max(0, n - 1));
}

TreeNode* Tree::selectedNode() {
    if (rows_.empty()) return nullptr;
    return rows_[selectedRow_];
}

void Tree::expand() {
    if (TreeNode* n = selectedNode(); n && !n->children.empty()) {
        n->expanded = true;
        rebuildRows();
    }
}

void Tree::collapse() {
    if (TreeNode* n = selectedNode()) {
        n->expanded = false;
        rebuildRows();
    }
}

void Tree::toggleExpand() {
    if (TreeNode* n = selectedNode(); n && !n->children.empty()) {
        n->expanded = !n->expanded;
        rebuildRows();
    }
}

void Tree::walkAll(const std::function<void(TreeNode*)>& fn) {
    std::function<void(TreeNode*)> walk = [&](TreeNode* n) {
        fn(n);
        for (TreeNode* child : n->children) walk(child);
    };
    for (TreeNode* root : roots_) walk(root);
}

void Tree::expandAll() {
    walkAll([](TreeNode* n) { n->expanded = !n->children.empty(); });
    rebuildRows();
}

void Tree::collapseAll() {
    walkAll([](TreeNode* n) { n->expanded = false; });
    rebuildRows();
}

void Tree::draw(Buffer& buf) {
    Block::draw(buf);

    int n = static_cast<int>(rows_.size());

    // Keep selected row in view.
    if (selectedRow_ >= topRow_ + inner.dy())
        topRow_ = selectedRow_ - inner.dy() + 1;
    else if (selectedRow_ < topRow_)
        topRow_ = selectedRow_;

    for (int ri = topRow_; ri < n && ri - topRow_ < inner.dy(); ++ri) {
        TreeNode* node = rows_[ri];
        int y = inner.min.y + (ri - topRow_);
        int x = inner.min.x + node->level * INDENT;

        Style style = (ri == selectedRow_) ? selectedRowStyle : textStyle;

        // Expand/collapse indicator for nodes that have children.
        if (!node->children.empty()) {
            char32_t indicator = node->expanded
                ? Theme.tree.expanded
                : Theme.tree.collapsed;
            if (x < inner.max.x) {
                buf.setCell(newCell(indicator, style), {x, y});
                ++x;
                buf.setCell(newCell(U' ', style), {x, y});
                ++x;
            }
        } else {
            // Leaf node: indent by 2 extra to align with parent text.
            x += 2;
        }

        // Draw the label, truncated if needed.
        for (char ch : node->label) {
            if (x >= inner.max.x) {
                buf.setCell(newCell(ELLIPSES, style), {inner.max.x - 1, y});
                break;
            }
            buf.setCell(newCell(static_cast<char32_t>(ch), style), {x, y});
            ++x;
        }
    }

    // Scroll arrows.
    if (topRow_ > 0)
        buf.setCell(newCell(UP_ARROW,   newStyle(ColorWhite)), {inner.max.x - 1, inner.min.y});
    if (n > topRow_ + inner.dy())
        buf.setCell(newCell(DOWN_ARROW, newStyle(ColorWhite)), {inner.max.x - 1, inner.max.y - 1});
}

} // namespace tui::widgets
