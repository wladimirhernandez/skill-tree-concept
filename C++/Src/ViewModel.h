#ifndef __SKILLTREE_VIEWMODEL_H__
#define __SKILLTREE_VIEWMODEL_H__


#include <NsCore/Noesis.h>
#include <NsCore/Ptr.h>
#include <NsCore/String.h>
#include <NsCore/Vector.h>
#include <NsCore/ReflectionDeclareEnum.h>
#include <NsGui/ObservableCollection.h>
#include <NsApp/NotifyPropertyChangedBase.h>
#include <NsApp/DelegateCommand.h>


namespace SkillTree
{

class ViewModel;

/// Visual + gameplay state of a node. Names match the concept sheet:
/// Locked (red), Available ("UNLOCKED" in the sheet, white), Pending/Owned ("SELECTED", gold).
enum class NodeState
{
    Locked,     // No acquired neighbour yet
    Available,  // Can be bought with mind shards
    Pending,    // Bought this session, waiting for ENTER/APPLY (refundable)
    Owned       // Committed
};

/// State of the connection between two nodes, drives the link colour
enum class LinkState
{
    Locked,     // At least one end is locked
    Open,       // Reachable path (no locked end)
    Active      // Both ends acquired
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class SkillNode final: public NoesisApp::NotifyPropertyChangedBase
{
public:
    SkillNode(ViewModel* owner, const char* name, const char* icon, const char* description,
        int col, int row, int cost, NodeState initial);

    int GetCol() const { return _col; }
    int GetRow() const { return _row; }
    int GetCost() const { return _cost; }

    NodeState GetState() const { return _state; }
    void SetState(NodeState value);

    bool IsAcquired() const { return _state == NodeState::Pending || _state == NodeState::Owned; }

    /// Initially available nodes stay purchasable even without acquired neighbours
    bool IsBaseAvailable() const { return _baseAvailable; }

    const Noesis::Vector<SkillNode*>& GetNeighbors() const { return _neighbors; }
    void AddNeighbor(SkillNode* node) { _neighbors.PushBack(node); }

    /// Plays the "denied" shake. The pulse alternates "A"/"B" so a DataTrigger fires on every call.
    void Deny();

private:
    void OnToggle(Noesis::BaseComponent* param);

private:
    ViewModel* _owner;
    Noesis::String _name;
    Noesis::String _icon;
    Noesis::String _description;
    int _col;
    int _row;
    float _x;
    float _y;
    int _cost;
    bool _tooltipOnLeft;
    bool _baseAvailable;
    NodeState _state;
    Noesis::String _denyPulse;
    Noesis::Vector<SkillNode*> _neighbors;
    Noesis::Ptr<NoesisApp::DelegateCommand> _toggle;

    NS_DECLARE_REFLECTION(SkillNode, NotifyPropertyChangedBase)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class SkillLink final: public NoesisApp::NotifyPropertyChangedBase
{
public:
    SkillLink(SkillNode* a, SkillNode* b);

    LinkState GetState() const;
    void Refresh();

private:
    SkillNode* _a;
    SkillNode* _b;
    float _x1;
    float _y1;
    float _x2;
    float _y2;

    NS_DECLARE_REFLECTION(SkillLink, NotifyPropertyChangedBase)
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class ViewModel final: public NoesisApp::NotifyPropertyChangedBase
{
public:
    ViewModel();

    /// Called by a node when it is clicked: buys an available node or refunds a pending one
    void Toggle(SkillNode* node);

    /// Exposed so the launcher can bind ENTER / ESC on the window
    NoesisApp::DelegateCommand* GetApplyCommand() const { return _apply; }
    NoesisApp::DelegateCommand* GetCloseCommand() const { return _close; }

private:
    SkillNode* AddNode(const char* name, const char* icon, const char* description, int col,
        int row, int cost, NodeState initial);
    void Connect(SkillNode* a, SkillNode* b);

    void Buy(SkillNode* node);
    void Refund(SkillNode* node);
    void RefundUnsupported();
    void Recompute();

    void OnApply(Noesis::BaseComponent* param);
    void OnClose(Noesis::BaseComponent* param);

    int GetShards() const { return _shards; }
    void SetShards(int value);
    bool GetHasPending() const;

private:
    Noesis::Ptr<Noesis::ObservableCollection<SkillNode>> _nodes;
    Noesis::Ptr<Noesis::ObservableCollection<SkillLink>> _links;
    Noesis::Ptr<NoesisApp::DelegateCommand> _apply;
    Noesis::Ptr<NoesisApp::DelegateCommand> _close;
    int _shards;
    Noesis::String _shardsDenyPulse;

    NS_DECLARE_REFLECTION(ViewModel, NotifyPropertyChangedBase)
};

}

NS_DECLARE_REFLECTION_ENUM(SkillTree::NodeState)
NS_DECLARE_REFLECTION_ENUM(SkillTree::LinkState)


#endif
