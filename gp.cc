#include <algorithm>
using std::find;
#include <iostream>
using std::cout;
using std::cerr;
#include <sstream>
using std::ostringstream;
#include <string>
using std::string;
#include <vector>
using std::vector;

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
  ostringstream s;
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
  GKind getKind() const { return kind; }
  GNode *getNext() const { if (!isDone()) { return lChild; } return 0; }
  GNode *getSibling() const { return rChild; }
  bool isDone() const { return kind == End; }
  bool hasMore() const { return !isDone(); }
  string toString() const {
    ostringstream s;
    s << ::toString(kind);
    if (!lChild) {
      s << "()";
    } else if (!rChild) {
      s << "(" << lChild->toString() << ")";
    } else {
      s << "(" << lChild->toString() << ", " << rChild->toString() << ")";
    }
    return s.str();
  }
private:
  GKind kind;
  GNode *lChild;
  GNode *rChild;
};

GNode *GenWait() { return new GNode(Wait); }
GNode *GenEnd() { return new GNode(End); }
GNode *GenCut(GNode *lChild) { return new GNode(Cut, lChild); }
GNode *GenTInc(GNode *lChild) { return new GNode(TInc, lChild); }
GNode *GenTDec(GNode *lChild) { return new GNode(TDec, lChild); }
GNode *GenLInc(GNode *lChild) { return new GNode(LInc, lChild); }
GNode *GenLDec(GNode *lChild) { return new GNode(LDec, lChild); }
GNode *GenWInc(GNode *lChild) { return new GNode(WInc, lChild); }
GNode *GenWDec(GNode *lChild) { return new GNode(WDec, lChild); }
GNode *GenPar(GNode *lChild, GNode *rChild) { return new GNode(Par, lChild, rChild); }
GNode *GenSer(GNode *lChild, GNode *rChild) { return new GNode(Ser, lChild, rChild); }

class INode;
class ONode;
class PNode;

vector<INode *> INodes;
vector<ONode *> ONodes;
vector<PNode *> PNodes;

class INode : public vector<ONode *> {
public:
  INode(std::initializer_list<ONode *> oNodes);
  INode(vector<ONode *> const &oNodes);
  INode() { }
  virtual string Name() const {
    ostringstream s;
    auto i = find(INodes.cbegin(), INodes.cend(), this);
    if (i != INodes.cend()) {
      s << "INodes[" << i - INodes.begin() << "]";
    } else {
      s << "INode";
    }
    return s.str();
  }
  virtual string toString() const;
  void addOutputTo(ONode *oNode) {
    push_back(oNode);
  }
  void removeOutputTo(ONode *oNode) {
    auto o = find(begin(), end(), oNode);
    if (o != end()) {
      erase(o);
    }
  }
};

struct Input {
  Input(INode *_iNode, int _weight = 1) : iNode(_iNode), weight(_weight) { }
  bool operator==(Input const &that) const { return iNode == that.iNode; }
  bool operator==(INode const *&that) const { return iNode == that; }
  bool operator<(Input const &that) const { return iNode < that.iNode || (iNode == that.iNode && weight < that.weight); }
  bool operator<(INode const *&that) const { return iNode < that; }
  void addOutputTo(ONode *oNode) { iNode->addOutputTo(oNode); }
  void removeOutputTo(ONode *oNode) { iNode->removeOutputTo(oNode); }
  string Name() const { return iNode->Name(); }
  string toString() const { return iNode->toString(); }
  
  INode *iNode;
  int weight;
};

class ONode : public vector<Input> {
public:
  ONode(std::initializer_list<INode *> iNodes, int _iLink = 0);
  ONode(vector<INode *> const &iNodes, int _iLink = 0);
  ONode(std::initializer_list<Input> iNodes, int _iLink = 0);
  ONode() : iLink(0) { }
  virtual string Name() const {
    ostringstream s;
    auto i = find(ONodes.cbegin(), ONodes.cend(), this);
    if (i != ONodes.cend()) {
      s << "ONodes[" << i - ONodes.begin() << "]";
    } else {
      s << "ONode";
    }
    return s.str();
  }
  virtual string toString() const;
  void addInputFrom(INode *iNode) {
    push_back(iNode);
  }
  void removeInputFrom(INode *iNode) {
    auto i = find(begin(), end(), iNode);
    if (i != end()) {
      erase(i);
    }
  }
  size_t iLinkAsIndex() const {
    if (iLink < 0) {
      return (iLink % size()) + size();
    }
    return 0 < size() ? iLink % size() : 0;
  }
  void bumpInputWeight(int by) {
    Input &input = (*this)[iLinkAsIndex()];
    input.weight += by;
  }
  void cutINode() {
    size_t i = iLinkAsIndex();
    Input &input = (*this)[i];
    INode *iNode = input.iNode;
    iNode->removeOutputTo(this);
    erase(begin() + i);
  }

protected:
  int iLink;
};

class PNode : public INode, public ONode {
public:
  PNode(std::initializer_list<ONode *> oNodes,
	std::initializer_list<INode *> iNodes,
	int _iLink = 0,
	int _threshold = 0,
	GNode *_genome = 0,
	GNode *_genomeReader = 0
       ) :
    INode(oNodes),
    ONode(iNodes, _iLink),
    threshold(_threshold),
    genome(_genome),
    genomeReader(_genomeReader)
  {
  }
  PNode(vector<ONode *> const &oNodes,
	vector<INode *> const &iNodes,
	int _iLink = 0,
	int _threshold = 0,
	GNode *_genome = 0,
	GNode *_genomeReader = 0
       ) :
    INode(oNodes),
    ONode(iNodes, _iLink),
    threshold(_threshold),
    genome(_genome),
    genomeReader(_genomeReader)
  {
  }
  PNode(std::initializer_list<ONode *> oNodes,
	std::initializer_list<Input> iNodes,
	int _iLink = 0,
	int _threshold = 0,
	GNode *_genome = 0,
	GNode *_genomeReader = 0
       ) :
    INode(oNodes),
    ONode(iNodes, _iLink),
    threshold(_threshold),
    genome(_genome),
    genomeReader(_genomeReader)
  {
  }
  virtual string Name() const {
    ostringstream s;
    auto i = find(PNodes.cbegin(), PNodes.cend(), this);
    if (i != PNodes.cend()) {
      s << "PNodes[" << i - PNodes.begin() << "]";
    } else {
      s << "PNode";
    }
    return s.str();
  }
  string toString() const {
    ostringstream s;
    s << Name() << ": { { outputNodes = {";
    for (auto n = INode::cbegin(); n != INode::cend(); n++) {
      if (n != INode::cbegin()) {
	s << ",";
      }
      s << " " << (*n)->Name();
    }
    s << " } }, { inputNodes = {";
    int i = iLinkAsIndex();
    for (auto n = ONode::cbegin(); n != ONode::cend(); n++) {
      if (n != ONode::cbegin()) {
	s << ",";
      }
      if (i-- == 0) {
	s << " <" << n->Name() << "(" << n->weight << ")>";
      } else {
	s << " " << n->Name() << "(" << n->weight << ")";
      }
    }
    s << " }";
    s << ", iLink = " << iLink << " }";
    s << ", threshold = " << threshold;
    s << ", genomeReader = " << genomeReader->toString();
    s << " }";
    return s.str();
  }
  void addInputFrom(Input iNode) {
    ONode::push_back(iNode);
    iNode.addOutputTo(this);
  }
  void addOutputTo(ONode *oNode) {
    INode::push_back(oNode);
    oNode->addInputFrom(this);
  }
  GKind getKind() const { return genomeReader->getKind(); }
  bool Grow() {
    switch (genomeReader->getKind()) {
    case Ser:
      splitSerially();
      genomeReader = genomeReader->getNext();
      break;
    case Par:
      splitParallel();
      genomeReader = genomeReader->getNext();
      break;
    case LInc:
      bumpInputLink(+1);
      genomeReader = genomeReader->getNext();
      break;
    case LDec:
      bumpInputLink(-1);
      genomeReader = genomeReader->getNext();
      break;
    case WInc:
      bumpInputWeight(+1);
      genomeReader = genomeReader->getNext();
      break;
    case WDec:
      bumpInputWeight(-1);
      genomeReader = genomeReader->getNext();
      break;
    case TInc:
      bumpThreshold(+1);
      genomeReader = genomeReader->getNext();
      break;
    case TDec:
      bumpThreshold(-1);
      genomeReader = genomeReader->getNext();
      break;
    case Cut:
      cutINode();
      genomeReader = genomeReader->getNext();
      break;
    case Wait:
      wait();
      genomeReader = genomeReader->getNext();
      break;
    case End:
      done();
      break;
    }

    return isDone();
  }
  void splitSerially() {
    PNode *sibling = new PNode();
    PNodes.push_back(sibling);

    // We'll keep our inputs, our sibling will get our outputs, and we
    // and our sibling will be connected, via our output to its input.

    for (auto n = INode::begin(); n != INode::end(); n++) {
      sibling->addOutputTo(*n);
      (*n)->removeInputFrom(this);
    }
    INode::clear();

    addOutputTo(sibling);
    sibling->iLink = 0;
    sibling->threshold = threshold;
    sibling->genome = genome;
    sibling->genomeReader = genomeReader->getSibling();

    // cout << "PNode::cloneSerially():\n";
    // cout << "    this->" << toString() << "\n";
    // cout << "    sibling->" << toString() << "\n";
  }
  void splitParallel() {
    PNode *sibling = new PNode();
    PNodes.push_back(sibling);

    // We'll keep our inputs and outputs, our sibling will get a copy
    // of both.

    for (auto n = INode::cbegin(); n != INode::cend(); n++) {
      sibling->addOutputTo(*n);
    }
    for (auto n = ONode::cbegin(); n != ONode::cend(); n++) {
      sibling->addInputFrom(*n);
    }
    sibling->iLink = iLink;
    sibling->threshold = threshold;
    sibling->genome = genome;
    sibling->genomeReader = genomeReader->getSibling();

    // cout << "PNode::cloneParallelly():\n";
    // cout << "    this->" << toString() << "\n";
    // cout << "    sibling->" << toString() << "\n";
  }
  void bumpInputLink(int by) {
    iLink += by;
  }
  void bumpInputWeight(int by) {
    ONode::bumpInputWeight(by);
  }
  void bumpThreshold(int by) {
    threshold += by;
  }
  void cutINode() {
    ONode::cutINode();
  }
  void wait() {
  }
  void done() {
  }
  bool isDone() const {
    return genomeReader->isDone();
  }
  bool hasMore() const {
    return genomeReader->hasMore();
  }

private:
  PNode() : threshold(0), genome(0), genomeReader(0) { }

  int threshold;
  GNode *genome;
  GNode *genomeReader;
};

INode::INode(std::initializer_list<ONode *> oNodes) {
  for (auto o = oNodes.begin(); o != oNodes.end(); o++) {
    push_back(*o);
    (*o)->addInputFrom(this);
  }
}

INode::INode(vector<ONode *> const &oNodes) {
  for (auto o = oNodes.begin(); o != oNodes.end(); o++) {
    push_back(*o);
    (*o)->addInputFrom(this);
  }
}

string INode::toString() const {
  ostringstream s;
  s << Name() << ": { outputNodes = {";
  for (auto n = cbegin(); n != cend(); n++) {
    if (n != cbegin()) {
      s << ",";
    }
    s << " " << (*n)->Name();
  }
  s << " } }";
  return s.str();
}

ONode::ONode(std::initializer_list<INode *> iNodes, int _iLink) :
  iLink(_iLink)
{
  for (auto i = iNodes.begin(); i != iNodes.end(); i++) {
    push_back({ *i, 1 });
    (*i)->addOutputTo(this);
  }
}

ONode::ONode(vector<INode *> const &iNodes, int _iLink) :
  iLink(_iLink)
{
  for (auto i = iNodes.begin(); i != iNodes.end(); i++) {
    push_back({ *i, 1 });
    (*i)->addOutputTo(this);
  }
}

ONode::ONode(std::initializer_list<Input> iNodes, int _iLink) :
  iLink(_iLink)
{
  for (auto i = iNodes.begin(); i != iNodes.end(); i++) {
    push_back(*i);
    i->iNode->addOutputTo(this);
  }
}

string ONode::toString() const {
  ostringstream s;
  s << Name() << ": { inputNodes = {";
  int i = iLinkAsIndex();
  for (auto n = cbegin(); n != cend(); n++) {
    if (n != cbegin()) {
      s << ",";
    }
    if (i-- == 0) {
      s << " <" << n->Name() << "(" << n->weight << ")>";
    } else {
      s << " " << n->Name() << "(" << n->weight << ")";
    }
  }
  s << " }, iLink = " << iLink << " }";
  return s.str();
}

void Dump() {
  cout << "Outputs: {\n";
  for (auto o = ONodes.cbegin(); o != ONodes.cend(); o++) {
    cout << "    " << (*o)->toString() << "\n";
  }
  cout << "}\n";

  cout << "Inputs: {\n";
  for (auto i = INodes.cbegin(); i != INodes.cend(); i++) {
    cout << "    " << (*i)->toString() << "\n";
  }
  cout << "}\n";

  cout << "Programs: {\n";
  for (auto p = PNodes.cbegin(); p != PNodes.cend(); p++) {
    cout << "    " << (*p)->toString() << "\n";
  }
  cout << "}\n";
}

int main(int argc, char const *argv[])
{
  GNode *genome =
    GenSer(GenPar(GenLInc(GenCut(GenEnd())),
		  GenCut(GenEnd())
		 ),
	   GenSer(GenPar(GenTInc(GenEnd()),
			 GenEnd()
			),
		  GenWDec(GenEnd())
		 )
	   );

  cout << "genome = " << genome->toString() << "\n";

  // ONode *oNode = new ONode();
  // ONodes.push_back(oNode);
  // 
  // INode *iNode = new INode();
  // INodes.push_back(iNode);
  // 
  // PNode *pNode = new PNode(ONodes, INodes, 0, 0, genome, genome);
  // PNodes.push_back(pNode);

  ONodes.push_back(new ONode());
  ONodes.push_back(new ONode());

  INodes.push_back(new INode());
  INodes.push_back(new INode());
  INodes.push_back(new INode());

  PNodes.push_back(new PNode(ONodes, INodes, 0, 0, genome, genome));
  
  Dump();

  bool isDone = false;
  for (size_t cycle = 0; !isDone; cycle += 1) {
    size_t hasMore = 0;
    size_t nPNodes = PNodes.size();

    for (size_t p = 0; p < nPNodes; p += 1) {
      PNode *pNode = PNodes[p];

      if (pNode->hasMore()) {
	cout << "# Growing by " << ::toString(pNode->getKind()) << ": " << pNode->toString() << "\n";

	if (!pNode->Grow()) {
	  hasMore += 1;
	}
      }
    }

    cout << "# " << cycle << " ------------------------------------------------------------------------\n";
    Dump();

    isDone = 0 == hasMore && nPNodes == PNodes.size();
  }

  return 0;
}
