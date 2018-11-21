#include <ostringstream>
using std::ostrstream;
#include <string>
using std::string;

enum GKind {
  Ser,
  Par,
  LInc,
  LDec,
  WInc,
  WDec,
  TInc,
  TDec,
  Cut,
  Wait,
  End,

  EoKind
};

string toString(GKind const &k) {
  switch (k) {
  case Ser: return "Ser";
  case Par: return "Par";
  case LInc: return "LInc";
  case LDec: return "LDec";
  case WInc: return "WInc";
  case WDec: return "WDec";
  case TInc: return "TInc";
  case TDec: return "TDec";
  case Cut: return "Cut";
  case Wait: return "Wait";
  case End: return "End";
  case EoKind: return "EoKind";
  }
  ostrstream s;
  s << "GKind(" << int(k) << ")";
  return s.str();
}

class GNode {
public:
  GNode(GKind _kind, GNode *_lChild = 0, GNode *_rChild = 0) :
    kind(_kind),
    lChild(_lChild),
    rChild(_rChild)
  {
  }
  GNode *next() const { return lChild; }
  bool isDone() const { return kind == End; }
  bool hasMore() const { return !isDone(); }
  string toString() const {
    ostrstream s = toString(kind);
    if (!lChild) {
      s << "()"
    } else if (!rChild) {
      s << "(" << lChild.toString() << ")";
    } else {
      s << "(" << lChild.toString() << ", " << rChild.toString() << ")";
    }
    return s.str();
  }
private:
  GKind kind;
  GNode *lChild;
  GNode *rChild;
};

GNode *Wait() { return new GNode(GNode::Wait); }
GNode *End() { return new GNode(GNode::End); }
GNode *Cut(GNode *lChild) { return new GNode(GNode::Cut, lChild); }
GNode *TInc(GNode *lChild) { return new GNode(GNode::TInc, lChild); }
GNode *TDec(GNode *lChild) { return new GNode(GNode::TDec, lChild); }
GNode *LInc(GNode *lChild) { return new GNode(GNode::LInc, lChild); }
GNode *LDec(GNode *lChild) { return new GNode(GNode::LDec, lChild); }
GNode *WInc(GNode *lChild) { return new GNode(GNode::WInc, lChild); }
GNode *WDec(GNode *lChild) { return new GNode(GNode::WDec, lChild); }
GNode *Par(GNode *lChild, GNode *rChild) { return new GNode(GNode::Par, lChild, rChild); }
GNode *Ser(GNode *lChild, GNode *rChild) { return new GNode(GNode::Ser, lChild, rChild); }

class InputNode;
class OutputNode;

class InputNode : public vector<OutputNode *> {
public:
  InputNode() { }
  void addOutputTo(OutputNode *oNode) {
    push_back(oNode);
  }
  void removeOutputTo(OutputNode *oNode) {
    auto o = find(begin(), end(), oNode);
    if (o != end()) {
      erase(o);
    }
  }
};

class OutputNode : public vector<Input> {
  struct Input {
    Input(InputNode *_iNode, int _weight = 1) : iNode(_iNode), weight(_weight) { }
    bool operator==(Input const &that) const { return iNode == that.iNode; }
    bool operator<(Input const &that) const { return iNode < that.iNode || (iNode == that.iNode && weight < that.weight); }

    InputNode *iNode;
    int weight;
  };

public:
  OutputNode() : inputLink(0) { }
  void addInputFrom(InputNode *iNode) {
    push_back(iNode);
  }
  void removeInputFrom(InputNode *iNode) {
    auto i = find(begin(), end(), iNode);
    if (i != end()) {
      erase(i);
    }
  }
  size_t inputLinkAsIndex() const {
    if (inputLink < 0) {
      return (inputLink % size()) + size();
    }
    return inputLink % size();
  }

private:
  int inputLink;
};

class INode;
class ONode;
class PNode;

vector<INode *> INodes;
vector<ONode *> ONodes;
vector<PNode *> PNodes;

class INode : public InputNode {
public:
  INode(std::initializer_list<ONode *> oNodes) : InputNode(oNodes) {
    for (auto o = begin(); o != end(); o++) {
      o->addInputFrom(this);
    }
  }
  string toString() const {
    ostrstream s;
    s << "{ outputNodes = {";
    for (auto n = cbegin(); n != cend(); n++) {
      auto p = find(PNodes.cbegin(); PNodes.cend(); *n);
      if (p != PNodes.cend()) {
	s << " PNodes[" << p - PNodes.begin() << "]";
      } else {
	auto o = find(ONodes.cbegin(); ONodes.cend(); *n);
	if (o != ONodes.cend()) {
	  s << " ONodes[" << o - ONodes.begin() << "]";
	}
      }

      if (n != cbegin()) {
	s << ",";
      }
    }
    s << " }";
    return s.str();
  }
};

class ONode : public OutputNode {
public:
  ONode(std::initializer_list<Input *> iNodes, iLink = 0) : OutputNode(iNodes, iLink) {
    for (auto i = begin(); i != end(); i++) {
      i->addOutputTo(this);
    }
  }
  string toString() const {
    ostrstream s;
    s << "{ inputNodes = {";
    for (auto n = cbegin(); n != cend(); n++) {
      auto p = find(PNodes.cbegin(); PNodes.cend(); *n);
      if (p != PNodes.cend()) {
	s << " PNodes[" << p - PNodes.begin() << "]";
      } else {
	auto i = find(INodes.cbegin(); INodes.cend(); *n);
	if (i != INodes.cend()) {
	  s << " INodes[" << i - INodes.begin() << "]";
	}
      }

      if (n != cbegin()) {
	s << ",";
      }
    }
    s << " }";
    return s.str();
  }
private:
};
