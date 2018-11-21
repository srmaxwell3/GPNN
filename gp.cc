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
  GNode *getNext() const { return lChild; }
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

class InputNode;
class OutputNode;

class InputNode : public vector<OutputNode *> {
public:
  InputNode(std::initializer_list<OutputNode *> oNodes) :
    vector<OutputNode *>(oNodes)
  {
  }
  InputNode(vector<OutputNode *> const &oNodes) :
    vector<OutputNode *>(oNodes)
  {
  }
  InputNode() { }
  virtual ~InputNode() { }
  virtual string Name() const { return "InputNode"; }
  virtual string toString() const = 0;
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

struct Input {
  Input(InputNode *_iNode, int _weight = 1) : iNode(_iNode), weight(_weight) { }
  bool operator==(Input const &that) const { return iNode == that.iNode; }
  bool operator==(InputNode const *&that) const { return iNode == that; }
  bool operator<(Input const &that) const { return iNode < that.iNode || (iNode == that.iNode && weight < that.weight); }
  bool operator<(InputNode const *&that) const { return iNode < that; }
  void addOutputTo(OutputNode *oNode) { iNode->addOutputTo(oNode); }
  void removeOutputTo(OutputNode *oNode) { iNode->removeOutputTo(oNode); }
  string Name() const { return iNode->Name(); }
  string toString() const { return iNode->toString(); }
  
  InputNode *iNode;
  int weight;
};

class OutputNode : public vector<Input> {
public:
  OutputNode(std::initializer_list<InputNode *> iNodes, int iLink) :
    inputLink(iLink)
  {
    for (auto i = iNodes.begin(); i != iNodes.end(); i += 1) {
      push_back(Input(*i));
    }
  }
  OutputNode(vector<InputNode *> const &iNodes, int iLink) :
    inputLink(iLink)
  {
    for (auto i = iNodes.begin(); i != iNodes.end(); i += 1) {
      push_back(Input(*i));
    }
  }
  OutputNode(std::initializer_list<Input> iNodes, int iLink) :
    vector<Input>(iNodes),
    inputLink(iLink)
  {
  }
  OutputNode() : inputLink(0) { }
  virtual ~OutputNode() { }
  virtual string Name() const { return "OutputNode"; }
  virtual string toString() const = 0;
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
    return 0 < size() ? inputLink % size() : 0;
  }

protected:
  int inputLink;
};

class INode;
class ONode;
class PNode;

vector<InputNode *> INodes;
vector<OutputNode *> ONodes;
vector<PNode *> PNodes;

class INode : public InputNode {
public:
  INode(std::initializer_list<OutputNode *> oNodes);
  INode(vector<OutputNode *> const &oNodes);
  INode() : InputNode() { }
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
  string toString() const;
};

class ONode : public OutputNode {
public:
  ONode(std::initializer_list<InputNode *> iNodes, int iLink = 0);
  ONode(vector<InputNode *> const &iNodes, int iLink = 0);
  ONode(std::initializer_list<Input> iNodes, int iLink = 0);
  ONode() : OutputNode() { }
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
  string toString() const;
};

class PNode : public InputNode, public OutputNode {
public:
  PNode(std::initializer_list<OutputNode *> oNodes,
	std::initializer_list<InputNode *> iNodes,
	int iLink = 0,
	int _threshold = 0,
	GNode *_genome = 0,
	GNode *_genomeReader = 0
       ) :
    InputNode(oNodes),
    OutputNode(iNodes, iLink),
    threshold(_threshold),
    genome(_genome),
    genomeReader(_genomeReader)
  {
  }
  PNode(vector<OutputNode *> const &oNodes,
	vector<InputNode *> const &iNodes,
	int iLink = 0,
	int _threshold = 0,
	GNode *_genome = 0,
	GNode *_genomeReader = 0
       ) :
    InputNode(oNodes),
    OutputNode(iNodes, iLink),
    threshold(_threshold),
    genome(_genome),
    genomeReader(_genomeReader)
  {
  }
  PNode(std::initializer_list<OutputNode *> oNodes,
	std::initializer_list<Input> iNodes,
	int iLink = 0,
	int _threshold = 0,
	GNode *_genome = 0,
	GNode *_genomeReader = 0
       ) :
    InputNode(oNodes),
    OutputNode(iNodes, iLink),
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
    s << Name() << ": {";
    s << " }";
    return s.str();
  }
  void addInputFrom(Input iNode) {
    OutputNode::push_back(iNode);
    iNode.addOutputTo(this);
  }
  void addOutputTo(OutputNode *oNode) {
    InputNode::push_back(oNode);
    oNode->addInputFrom(this);
  }
  bool Grow() {
    switch (genomeReader->getKind()) {
    case Ser:
      splitSerially();
      break;
    case Par:
      splitParallel();
      break;
    case LInc:
      bumpInputLink(+1);
      break;
    case LDec:
      bumpInputLink(-1);
      break;
    case WInc:
      bumpInputWeight(+1);
      break;
    case WDec:
      bumpInputWeight(-1);
      break;
    case TInc:
      bumpThreshold(+1);
      break;
    case TDec:
      bumpThreshold(-1);
      break;
    case Cut:
      cutInputNode();
      break;
    case Wait:
      wait();
      break;
    case End:
      done();
      break;
    }
    return isDone();
  }
  void splitSerially() {
  }
  void splitParallel() {
  }
  void bumpInputLink(int by) {
  }
  void bumpInputWeight(int by) {
  }
  void bumpThreshold(int by) {
  }
  void cutInputNode() {
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
  int threshold;
  GNode *genome;
  GNode *genomeReader;
};

INode::INode(std::initializer_list<OutputNode *> oNodes) : InputNode(oNodes) {
  for (auto o = begin(); o != end(); o++) {
    (*o)->addInputFrom(this);
  }
}

INode::INode(vector<OutputNode *> const &oNodes) : InputNode(oNodes) {
  for (auto o = begin(); o != end(); o++) {
    (*o)->addInputFrom(this);
  }
}

string INode::toString() const {
  ostringstream s;
  s << Name() << ": { outputNodes = {";
  for (auto n = cbegin(); n != cend(); n++) {
    s << " " << (*n)->Name();

    if (n != cbegin()) {
      s << ",";
    }
  }
  s << " } }";
  return s.str();
}

ONode::ONode(std::initializer_list<InputNode *> iNodes, int iLink) :
  OutputNode(iNodes, iLink)
{
  for (auto i = begin(); i != end(); i++) {
    i->addOutputTo(this);
  }
}

ONode::ONode(vector<InputNode *> const &iNodes, int iLink) :
  OutputNode(iNodes, iLink)
{
  for (auto i = begin(); i != end(); i++) {
    i->addOutputTo(this);
  }
}

ONode::ONode(std::initializer_list<Input> iNodes, int iLink) :
  OutputNode(iNodes, iLink)
{
  for (auto i = begin(); i != end(); i++) {
    i->addOutputTo(this);
  }
}

string ONode::toString() const {
  ostringstream s;
  s << Name() << ": { inputNodes = {";
  for (auto n = cbegin(); n != cend(); n++) {
    s << " " << n->Name();

    if (n != cbegin()) {
      s << ",";
    }
  }
  s << " }, inputLink = " << inputLink << " }";
  return s.str();
}

int main(int argc, char const *argv[])
{
  GNode *genome =
    GenSer(GenPar(GenEnd(),
		  GenEnd()
		 ),
	   GenSer(GenPar(GenTInc(GenEnd()),
			 GenEnd()
			),
		  GenWDec(GenEnd())
		 )
	   );

  cout << "genome = " << genome->toString() << "\n";

  ONode *oNode = new ONode();
  ONodes.push_back(oNode);

  INode *iNode = new INode();
  INodes.push_back(iNode);

  PNode *pNode = new PNode(ONodes, INodes, 0, 0, genome, genome);
  PNodes.push_back(pNode);

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

  cout << "Ancestor: " << pNode->toString() << "\n";
  return 0;
}
