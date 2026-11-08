#include "ViewModel.h"

#include <NsCore/ReflectionImplement.h>
#include <NsCore/ReflectionImplementEnum.h>
#include <NsCore/Log.h>
#include <NsApp/Application.h>


using namespace SkillTree;
using namespace Noesis;
using namespace NoesisApp;


namespace
{

// Node lattice, measured on the 1920x1080 concept: x = 960 + 156 * col, y = 540 + 90 * row
// (col + row is always even). Same constants as tools/gen_hex_grid.py.
constexpr float CenterX = 960.0f;
constexpr float CenterY = 540.0f;
constexpr float StepX = 156.0f;
constexpr float StepY = 90.0f;

// Nodes from this column on show their tooltip on the left so it stays on screen
constexpr int TooltipFlipColumn = 3;

constexpr int StartingShards = 3;

float ToX(int col) { return CenterX + StepX * (float)col; }
float ToY(int row) { return CenterY + StepY * (float)row; }

bool Contains(const Vector<SkillNode*>& v, const SkillNode* node)
{
    for (const SkillNode* n : v)
    {
        if (n == node) return true;
    }
    return false;
}

}


////////////////////////////////////////////////////////////////////////////////////////////////////
SkillNode::SkillNode(ViewModel* owner, const char* name, const char* icon, const char* description,
    int col, int row, int cost, NodeState initial): _owner(owner), _name(name), _icon(icon),
    _description(description), _col(col), _row(row), _x(ToX(col)), _y(ToY(row)), _cost(cost),
    _tooltipOnLeft(col >= TooltipFlipColumn), _baseAvailable(initial != NodeState::Locked),
    _state(initial)
{
    _toggle = MakePtr<DelegateCommand>(MakeDelegate(this, &SkillNode::OnToggle));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SkillNode::SetState(NodeState value)
{
    if (_state != value)
    {
        _state = value;
        OnPropertyChanged("State");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SkillNode::Deny()
{
    _denyPulse = _denyPulse == "A" ? "B" : "A";
    OnPropertyChanged("DenyPulse");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SkillNode::OnToggle(BaseComponent*)
{
    _owner->Toggle(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SkillLink::SkillLink(SkillNode* a, SkillNode* b): _a(a), _b(b), _x1(ToX(a->GetCol())),
    _y1(ToY(a->GetRow())), _x2(ToX(b->GetCol())), _y2(ToY(b->GetRow()))
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
LinkState SkillLink::GetState() const
{
    if (_a->GetState() == NodeState::Locked || _b->GetState() == NodeState::Locked)
    {
        return LinkState::Locked;
    }

    if (_a->IsAcquired() && _b->IsAcquired())
    {
        return LinkState::Active;
    }

    return LinkState::Open;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void SkillLink::Refresh()
{
    OnPropertyChanged("State");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ViewModel::ViewModel(): _shards(StartingShards)
{
    _nodes = MakePtr<ObservableCollection<SkillNode>>();
    _links = MakePtr<ObservableCollection<SkillLink>>();
    _apply = MakePtr<DelegateCommand>(MakeDelegate(this, &ViewModel::OnApply));
    _close = MakePtr<DelegateCommand>(MakeDelegate(this, &ViewModel::OnClose));

    const NodeState L = NodeState::Locked;
    const NodeState A = NodeState::Available;
    const NodeState O = NodeState::Owned;

    // Core (owned at start, gold in the concept)
    SkillNode* lifeEssence = AddNode("LIFE ESSENCE", "Layers", "The root of your power. Every path begins here.", 0, 0, 1, O);
    SkillNode* vividLie = AddNode("VIVID LIE", "Slash", "Enemies hit by your illusions lose track of you for 2 seconds.", -1, -1, 1, O);
    SkillNode* divineHeart = AddNode("DIVINE HEART", "Funnel", "Ancient souls watch over you. Once per battle, you rise again after falling.", 1, -1, 1, O);
    SkillNode* orcAmok = AddNode("ORC AMOK", "Peak", "Each kill grants 5% attack speed, stacking up to 5 times.", -1, 1, 1, O);
    SkillNode* magsEye = AddNode("MAG'S EYE", "Spiral", "Reveals hidden enemies and weak points within 20 meters.", 1, 1, 1, O);
    SkillNode* neoVoid = AddNode("NEO VOID", "Rune", "Your dodge leaves a rift that slows nearby foes.", 0, 2, 1, O);

    // Inner ring (available at start, white)
    SkillNode* tricorn = AddNode("TRICORN", "Tri", "Your third consecutive strike always lands critical.", -2, -2, 1, A);
    SkillNode* foulSoul = AddNode("FOUL SOUL", "Ring", "Defeated enemies release a cloud that poisons their allies.", -3, -1, 1, A);
    SkillNode* noMercy = AddNode("NO MERCY", "Rune", "Deal 20% more damage to enemies below half health.", -4, -2, 1, A);
    SkillNode* cloud = AddNode("CLOUD", "Burst", "Gain a short burst of evasion after taking heavy damage.", 2, -2, 1, A);
    SkillNode* divided = AddNode("DIVIDED", "Tri", "Your projectiles split in two on their first impact.", 2, -4, 1, A);
    SkillNode* badCell = AddNode("BAD CELL", "Spiral", "Bleeding enemies take 10% more damage from all sources.", 3, -1, 1, A);
    SkillNode* cleanCut = AddNode("CLEAN CUT", "Ring", "Finishing blows restore a portion of your stamina.", -3, 1, 1, A);
    SkillNode* cryoBreath = AddNode("CRYO BREATH", "Spiral", "Your heavy attacks chill the target, reducing its speed.", -2, 2, 1, A);
    SkillNode* connection = AddNode("CONNECTION", "Burst", "Allies near you share 10% of the damage you receive.", -3, 3, 1, A);
    SkillNode* blindArrow = AddNode("BLIND ARROW", "Peak", "Arrows fired while unseen pierce through armour.", 2, 2, 1, A);
    SkillNode* flyingAnger = AddNode("FLYING ANGER", "Ring", "Airborne attacks build rage twice as fast.", 3, 3, 1, A);
    SkillNode* sundered = AddNode("SUNDERED", "Ring", "Breaking a shield stuns its bearer for 1 second.", 4, 4, 1, A);
    SkillNode* vitrium = AddNode("VITRIUM", "Funnel", "Crystallise your skin, blocking the next hit every 30 seconds.", 0, 4, 1, A);

    // Outer ring (locked at start, red)
    SkillNode* highOmen = AddNode("HIGH OMEN", "Ring", "See the next attack of the strongest enemy in the area.", -4, -4, 2, L);
    SkillNode* closeCall = AddNode("CLOSE CALL", "Tri", "Surviving a lethal hit grants 3 seconds of invulnerability.", -2, -4, 2, L);
    SkillNode* fireBurn = AddNode("FIRE BURN", "Layers", "Your strikes ignite oil, burning enemies over time.", -3, -3, 2, L);
    SkillNode* powerBoost = AddNode("POWER BOOST", "Burst", "Charged attacks deal double damage.", -1, -5, 2, L);
    SkillNode* dominant = AddNode("DOMINANT", "Layers", "Enemies with less health than you hesitate before attacking.", 1, -5, 2, L);
    SkillNode* alienSight = AddNode("ALIEN SIGHT", "Peak", "See through walls while standing still.", 4, -4, 2, L);
    SkillNode* highCrown = AddNode("HIGH CROWN", "Spiral", "Leadership aura: allies deal 10% more damage.", 3, -3, 2, L);
    SkillNode* lifeFeed = AddNode("LIFE FEED", "Slash", "Heal for 5% of the damage you deal.", 4, -2, 2, L);
    SkillNode* vitalPoint = AddNode("VITAL POINT", "Layers", "Critical hits ignore 30% of the target's armour.", -4, 2, 2, L);
    SkillNode* deathItch = AddNode("DEATH ITCH", "Layers", "Gain damage the closer you are to death.", -4, 4, 2, L);
    SkillNode* crossMind = AddNode("CROSS MIND", "Spiral", "Swap places with a marked enemy.", -2, 4, 2, L);
    SkillNode* aero = AddNode("AERO", "Peak", "Double jump and glide for a short distance.", -1, 5, 2, L);
    SkillNode* coreCrob = AddNode("CORE CROB", "Spiral", "Your armour regenerates out of combat.", 3, 1, 2, L);
    SkillNode* smithing = AddNode("SMITHING", "Tri", "Repair your gear at any campfire.", 4, 2, 2, L);
    SkillNode* vileVenom = AddNode("VILE VENOM", "Layers", "Poison effects spread to nearby enemies.", 2, 4, 2, L);
    SkillNode* crimsonCry = AddNode("CRIMSON CRY", "Spiral", "Your war cry makes enemies bleed.", 1, 5, 2, L);

    // Connections, as drawn in the concept
    Connect(lifeEssence, vividLie);
    Connect(lifeEssence, divineHeart);
    Connect(lifeEssence, orcAmok);
    Connect(lifeEssence, magsEye);
    Connect(lifeEssence, neoVoid);

    Connect(vividLie, tricorn);
    Connect(tricorn, foulSoul);
    Connect(tricorn, fireBurn);
    Connect(tricorn, closeCall);
    Connect(foulSoul, noMercy);
    Connect(fireBurn, highOmen);
    Connect(closeCall, powerBoost);

    Connect(divineHeart, cloud);
    Connect(cloud, divided);
    Connect(cloud, highCrown);
    Connect(cloud, badCell);
    Connect(divided, dominant);
    Connect(highCrown, alienSight);
    Connect(badCell, lifeFeed);

    Connect(orcAmok, cryoBreath);
    Connect(cryoBreath, cleanCut);
    Connect(cryoBreath, connection);
    Connect(cryoBreath, crossMind);
    Connect(cleanCut, vitalPoint);
    Connect(connection, deathItch);
    Connect(crossMind, aero);

    Connect(magsEye, blindArrow);
    Connect(blindArrow, coreCrob);
    Connect(blindArrow, flyingAnger);
    Connect(blindArrow, vileVenom);
    Connect(coreCrob, smithing);
    Connect(flyingAnger, sundered);
    Connect(vileVenom, crimsonCry);

    Connect(neoVoid, vitrium);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SkillNode* ViewModel::AddNode(const char* name, const char* icon, const char* description, int col,
    int row, int cost, NodeState initial)
{
    Ptr<SkillNode> node = MakePtr<SkillNode>(this, name, icon, description, col, row, cost, initial);
    _nodes->Add(node);
    return node;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::Connect(SkillNode* a, SkillNode* b)
{
    a->AddNeighbor(b);
    b->AddNeighbor(a);
    _links->Add(MakePtr<SkillLink>(a, b));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::Toggle(SkillNode* node)
{
    switch (node->GetState())
    {
        case NodeState::Available: Buy(node); break;
        case NodeState::Pending: Refund(node); break;
        case NodeState::Locked: node->Deny(); break;
        case NodeState::Owned: break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::Buy(SkillNode* node)
{
    if (_shards < node->GetCost())
    {
        NS_LOG_INFO("Not enough mind shards (have %d, need %d)", _shards, node->GetCost());
        node->Deny();
        _shardsDenyPulse = _shardsDenyPulse == "A" ? "B" : "A";
        OnPropertyChanged("ShardsDenyPulse");
        return;
    }

    SetShards(_shards - node->GetCost());
    node->SetState(NodeState::Pending);
    Recompute();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::Refund(SkillNode* node)
{
    SetShards(_shards + node->GetCost());
    node->SetState(NodeState::Available);

    // Pending nodes that were only reachable through this one are refunded too
    RefundUnsupported();
    Recompute();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::RefundUnsupported()
{
    // Flood fill from every committed node and every initially available pending node,
    // walking only through pending nodes. Pending nodes not reached lost their support.
    Vector<SkillNode*> reached;
    Vector<SkillNode*> queue;

    for (int i = 0; i < _nodes->Count(); i++)
    {
        SkillNode* n = _nodes->Get(i);
        bool root = n->GetState() == NodeState::Owned ||
            (n->GetState() == NodeState::Pending && n->IsBaseAvailable());

        if (root)
        {
            reached.PushBack(n);
            queue.PushBack(n);
        }
    }

    for (uint32_t i = 0; i < queue.Size(); i++)
    {
        for (SkillNode* neighbor : queue[i]->GetNeighbors())
        {
            if (neighbor->GetState() == NodeState::Pending && !Contains(reached, neighbor))
            {
                reached.PushBack(neighbor);
                queue.PushBack(neighbor);
            }
        }
    }

    for (int i = 0; i < _nodes->Count(); i++)
    {
        SkillNode* n = _nodes->Get(i);
        if (n->GetState() == NodeState::Pending && !Contains(reached, n))
        {
            SetShards(_shards + n->GetCost());
            n->SetState(NodeState::Available);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::Recompute()
{
    for (int i = 0; i < _nodes->Count(); i++)
    {
        SkillNode* n = _nodes->Get(i);
        if (n->IsAcquired())
        {
            continue;
        }

        bool reachable = n->IsBaseAvailable();
        for (SkillNode* neighbor : n->GetNeighbors())
        {
            reachable = reachable || neighbor->IsAcquired();
        }

        n->SetState(reachable ? NodeState::Available : NodeState::Locked);
    }

    for (int i = 0; i < _links->Count(); i++)
    {
        _links->Get(i)->Refresh();
    }

    OnPropertyChanged("HasPending");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::OnApply(BaseComponent*)
{
    for (int i = 0; i < _nodes->Count(); i++)
    {
        SkillNode* n = _nodes->Get(i);
        if (n->GetState() == NodeState::Pending)
        {
            n->SetState(NodeState::Owned);
        }
    }

    Recompute();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::OnClose(BaseComponent*)
{
    if (!GetHasPending())
    {
        Application::Current()->Shutdown();
        return;
    }

    // First ESC discards the pending selection, second ESC closes
    for (int i = 0; i < _nodes->Count(); i++)
    {
        SkillNode* n = _nodes->Get(i);
        if (n->GetState() == NodeState::Pending)
        {
            SetShards(_shards + n->GetCost());
            n->SetState(NodeState::Available);
        }
    }

    Recompute();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void ViewModel::SetShards(int value)
{
    if (_shards != value)
    {
        _shards = value;
        OnPropertyChanged("Shards");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool ViewModel::GetHasPending() const
{
    for (int i = 0; i < _nodes->Count(); i++)
    {
        if (_nodes->Get(i)->GetState() == NodeState::Pending)
        {
            return true;
        }
    }

    return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
NS_BEGIN_COLD_REGION

NS_IMPLEMENT_REFLECTION(SkillNode)
{
    NsProp("Name", &SkillNode::_name);
    NsProp("Icon", &SkillNode::_icon);
    NsProp("Description", &SkillNode::_description);
    NsProp("X", &SkillNode::_x);
    NsProp("Y", &SkillNode::_y);
    NsProp("Cost", &SkillNode::_cost);
    NsProp("TooltipOnLeft", &SkillNode::_tooltipOnLeft);
    NsProp("State", &SkillNode::GetState);
    NsProp("DenyPulse", &SkillNode::_denyPulse);
    NsProp("Toggle", &SkillNode::_toggle);
}

NS_IMPLEMENT_REFLECTION(SkillLink)
{
    NsProp("X1", &SkillLink::_x1);
    NsProp("Y1", &SkillLink::_y1);
    NsProp("X2", &SkillLink::_x2);
    NsProp("Y2", &SkillLink::_y2);
    NsProp("State", &SkillLink::GetState);
}

NS_IMPLEMENT_REFLECTION(ViewModel)
{
    NsProp("Nodes", &ViewModel::_nodes);
    NsProp("Links", &ViewModel::_links);
    NsProp("Shards", &ViewModel::GetShards);
    NsProp("HasPending", &ViewModel::GetHasPending);
    NsProp("ShardsDenyPulse", &ViewModel::_shardsDenyPulse);
    NsProp("Apply", &ViewModel::_apply);
    NsProp("Close", &ViewModel::_close);
}

NS_IMPLEMENT_REFLECTION_ENUM(NodeState)
{
    NsVal("Locked", NodeState::Locked);
    NsVal("Available", NodeState::Available);
    NsVal("Pending", NodeState::Pending);
    NsVal("Owned", NodeState::Owned);
}

NS_IMPLEMENT_REFLECTION_ENUM(LinkState)
{
    NsVal("Locked", LinkState::Locked);
    NsVal("Open", LinkState::Open);
    NsVal("Active", LinkState::Active);
}

NS_END_COLD_REGION
