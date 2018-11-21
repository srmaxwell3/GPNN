#include <algorithm>
using std::find;
using std::swap;
#include <cassert>
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
  IInc,
  IDec,
  ICut,
  OInc,
  ODec,
  OCut,
  WInc,
  WDec,
  WShl,
  WShr,
  TInc,
  TDec,
  TShl,
  TShr,
  Wait,
  End,

  EoKind
};

string toString(GKind const &k) {
  switch (k) {
  case Ser: return "Ser";
  case Par: return "Par";
  case IInc: return "IInc";
  case IDec: return "IDec";
  case ICut: return "ICut";
  case OInc: return "OInc";
  case ODec: return "ODec";
  case OCut: return "OCut";
  case WInc: return "WInc";
  case WDec: return "WDec";
  case WShl: return "WShl";
  case WShr: return "WShr";
  case TInc: return "TInc";
  case TDec: return "TDec";
  case TShl: return "TShl";
  case TShr: return "TShr";
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
GNode *GenTInc(GNode *lChild) { return new GNode(TInc, lChild); }
GNode *GenTDec(GNode *lChild) { return new GNode(TDec, lChild); }
GNode *GenIInc(GNode *lChild) { return new GNode(IInc, lChild); }
GNode *GenIDec(GNode *lChild) { return new GNode(IDec, lChild); }
GNode *GenICut(GNode *lChild) { return new GNode(ICut, lChild); }
GNode *GenOInc(GNode *lChild) { return new GNode(OInc, lChild); }
GNode *GenODec(GNode *lChild) { return new GNode(ODec, lChild); }
GNode *GenOCut(GNode *lChild) { return new GNode(OCut, lChild); }
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
  INode(std::initializer_list<ONode *> oNodes, int _oLink = 0);
  INode(vector<ONode *> const &oNodes, int _oLink = 0);
  INode() : oLink(0) { }
  virtual ~INode();
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
  size_t oLinkAsIndex() const {
    if (int s = size()) {
      int o = oLink;
      while (o < 0) {
	o += s;
      }
      while (s <= o) {
	o -= s;
      }
      return o;
    }
    return 0;
  }
  void cutONode();
  void setValue(int _value) {
    evaluated = true;
    value = _value;
  }
  void Reset() { evaluated = false; }
  virtual bool isEvaluated() const { return evaluated; }
  virtual int Evaluate() {
    evaluated = true;
    // cout << Name() << "->Evaluate() = " << value << "\n";
    return value;
  }

protected:
  int oLink;
  bool evaluated;
  int value;
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
  void Reset() { iNode->Reset(); }
  bool isEvaluated() const { return iNode->isEvaluated(); }
  int Evaluate() {
    // cout << Name() << "->Evaluate() = " << iNode->Evaluate() << " * " << weight << "\n";
    return iNode->Evaluate() * weight;
  }

  INode *iNode;
  int weight;
};

class ONode : public vector<Input> {
public:
  ONode(std::initializer_list<INode *> iNodes, int _iLink = 0);
  ONode(vector<INode *> const &iNodes, int _iLink = 0);
  ONode(std::initializer_list<Input> iNodes, int _iLink = 0);
  ONode() : iLink(0), evaluated(false), value(0) { }
  virtual ~ONode() {
    while (!empty()) {
      back().iNode->removeOutputTo(this);
      pop_back();
    }
  }
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
    if (int s = size()) {
      int i = iLink;
      while (i < 0) {
	i += s;
      }
      while (s <= i) {
	i -= s;
      }
      return i;
    }
    return 0;
  }
  void bumpInputWeight(int by) {
    if (empty()) {
      return;
    }

    Input &input = (*this)[iLinkAsIndex()];
    input.weight += by;
  }
  void shiftInputWeight(int by) {
    if (empty()) {
      return;
    }

    Input &input = (*this)[iLinkAsIndex()];
    input.weight << by;
  }
  void cutINode() {
    if (empty()) {
      return;
    }

    size_t i = iLinkAsIndex();

    if ((i + 1) < size()) {
      std::swap((*this)[i], back());
    }

    INode *iNode = back().iNode;
    iNode->removeOutputTo(this);
    pop_back();
  }
  void Reset() {
    if (isEvaluated()) {
      evaluated = false;
      for (auto i = begin(); i != end(); i++) {
	i->Reset();
      }
    }
  }
  bool isEvaluated() const { return evaluated; }
  int Evaluate() {
    if (!isEvaluated()) {
      // cout << Name() << "->Evaluate()...\n";
      value = 0;
      for (auto i = begin(); i != end(); i++) {
	value += i->Evaluate();
        // cout << Name() << "->Evaluate() = " << value << "...\n";
      }
      evaluated = true;
    }
    return value;
  }

protected:
  int iLink;
  bool evaluated;
  int value;
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
  virtual ~PNode() {
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
    if (isEvaluated()) {
      s << " value = " << value;
    } else {
      s << " value = ???";
    }
    s << ", { outputNodes = {";
    int o = oLinkAsIndex();
    for (auto n = INode::cbegin(); n != INode::cend(); n++) {
      if (n != INode::cbegin()) {
	s << ",";
      }
      if (o-- == 0) {
	s << " <" << (*n)->Name() << ">";
      } else {
	s << " " << (*n)->Name();
      }
    }
    s << " }, oLink = " << oLink;
    s << " }, {";
    s << " value = " << ONode::value << (ONode::evaluated ? "" : "???");
    s << ", inputNodes = {";
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
    s << " }, iLink = " << iLink;
    s << " }";
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
    GKind kind = genomeReader->getKind();

    switch (kind) {
    case Ser:
      splitSerially();
      break;
    case Par:
      splitParallel();
      break;
    case IInc:
      bumpILink(+1);
      break;
    case IDec:
      bumpILink(-1);
      break;
    case ICut:
      cutINode();
      break;
    case OInc:
      bumpOLink(+1);
      break;
    case ODec:
      bumpOLink(-1);
      break;
    case OCut:
      cutONode();
      break;
    case WInc:
      bumpInputWeight(+1);
      break;
    case WDec:
      bumpInputWeight(-1);
      break;
    case WShl:
      shiftInputWeight(+1);
      break;
    case WShr:
      shiftInputWeight(-1);
      break;
    case TInc:
      bumpThreshold(+1);
      break;
    case TDec:
      bumpThreshold(-1);
      break;
    case TShl:
      shiftThreshold(+1);
      break;
    case TShr:
      shiftThreshold(-1);
      break;
    case Wait:
      wait();
      break;
    case End:
      done();
      break;
    }

    if (kind != End) {
      genomeReader = genomeReader->getNext();
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
  void bumpILink(int by) {
    iLink += by;
  }
  void bumpOLink(int by) {
    oLink += by;
  }
  void bumpInputWeight(int by) {
    ONode::bumpInputWeight(by);
  }
  void shiftInputWeight(int by) {
    ONode::shiftInputWeight(by);
  }
  void bumpThreshold(int by) {
    threshold += by;
  }
  void shiftThreshold(int by) {
    threshold << by;
  }
  void cutINode() {
    ONode::cutINode();
  }
  void cutONode() {
    INode::cutONode();
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
  void Reset() { ONode::Reset(); }
  virtual bool isEvaluated() const { return ONode::isEvaluated(); }
  virtual int Evaluate() {
    if (!isEvaluated()) {
      int oValue = ONode::Evaluate();

      if (threshold < 0) {
        value = (oValue < threshold) ? oValue - threshold : threshold;
      } else {
        value = (threshold < oValue) ? oValue - threshold : threshold;
      }
      // cout << Name() << "->Evaluate() = " << value << " - " << threshold << "\n";
      value = oValue - threshold;
    }
    return value;
  }

private:
  PNode() : threshold(0), genome(0), genomeReader(0) { }

  int threshold;
  int value;
  GNode *genome;
  GNode *genomeReader;
};

INode::INode(std::initializer_list<ONode *> oNodes, int _oLink) :
  oLink(_oLink)
{
  for (auto o = oNodes.begin(); o != oNodes.end(); o++) {
    push_back(*o);
    (*o)->addInputFrom(this);
  }
}

INode::INode(vector<ONode *> const &oNodes, int _oLink) :
  oLink(_oLink)
{
  for (auto o = oNodes.begin(); o != oNodes.end(); o++) {
    push_back(*o);
    (*o)->addInputFrom(this);
  }
}

INode::~INode()
{
  while (!empty()) {
    back()->removeInputFrom(this);
    pop_back();
  }
}

string INode::toString() const {
  ostringstream s;
  s << Name() << ": {";
  if (isEvaluated()) {
    s << " value = " << value;
  } else {
    s << ", value = ???";
  }
  s << ", outputNodes = {";
  int o = oLinkAsIndex();
  for (auto n = cbegin(); n != cend(); n++) {
    if (n != cbegin()) {
      s << ",";
    }
    if (o-- == 0) {
      s << " <" << (*n)->Name() << ">";
    } else {
      s << " " << (*n)->Name();
    }
  }
  s << " }, oLink = " << oLink;
  s << " } }";
  return s.str();
}

void INode::cutONode() {
  if (empty()) {
    return;
  }

  size_t o = oLinkAsIndex();

  if ((o + 1) < size()) {
    std::swap((*this)[o], back());
  }

  ONode *oNode = back();
  oNode->removeInputFrom(this);
  pop_back();
}

ONode::ONode(std::initializer_list<INode *> iNodes, int _iLink) :
  iLink(_iLink), evaluated(false), value(0)
{
  for (auto i = iNodes.begin(); i != iNodes.end(); i++) {
    push_back({ *i, 1 });
    (*i)->addOutputTo(this);
  }
}

ONode::ONode(vector<INode *> const &iNodes, int _iLink) :
  iLink(_iLink), evaluated(false), value(0)
{
  for (auto i = iNodes.begin(); i != iNodes.end(); i++) {
    push_back({ *i, 1 });
    (*i)->addOutputTo(this);
  }
}

ONode::ONode(std::initializer_list<Input> iNodes, int _iLink) :
  iLink(_iLink), evaluated(false), value(0)
{
  for (auto i = iNodes.begin(); i != iNodes.end(); i++) {
    push_back(*i);
    i->iNode->addOutputTo(this);
  }
}

string ONode::toString() const {
  ostringstream s;
  s << Name() << ": {";
  if (isEvaluated()) {
    s << " value = " << value;
  } else {
    s << " value = ???";
  }
  s << ", inputNodes = {";
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
  s << " }, iLink = " << iLink;
  s << " }";
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

static size_t builtNRandomNodes;

GNode *buildRandom() {
  static int likelihoods[EoKind] = {
     5, // 5, // Ser
     5, // 5, // Par
     2, // 2, // IInc
     2, // 2, // IDec
     1, // 2, // ICut
     2, // 2, // OInc
     2, // 2, // ODec
     1, // 2, // OCut
     2, // 2, // WInc
     2, // 2, // WDec
     2, // 2, // WShl
     2, // 2, // WShr
     2, // 2, // TInc
     2, // 2, // TDec
     2, // 2, // TShl
     2, // 2, // TShr
     2, // 2, // Wait
     0, // 1, // End
  };
  static int maxLikelihoods = 0;

  if (!maxLikelihoods) {
    for (auto k = Ser; k < EoKind; k = GKind(int(k) + 1)) {
      maxLikelihoods += likelihoods[k];
    }
  }

  if (++builtNRandomNodes < 100) {
    return new GNode(End);
  }

  int choose = rand() % maxLikelihoods;
  for (auto k = Ser; k < EoKind; k = GKind(int(k) + 1)) {
    choose -= likelihoods[k];
    if (choose <= 0) {
      switch (k) {
        case Ser:
        case Par:
          {
            GNode *lChild = buildRandom();
            GNode *rChild = buildRandom();
            return new GNode(k, lChild, rChild);
          }
        case IInc:
        case IDec:
        case ICut:
        case OInc:
        case ODec:
        case OCut:
        case WInc:
        case WDec:
        case WShl:
        case WShr:
        case TInc:
        case TDec:
        case TShl:
        case TShr:
        case Wait:
          {
            GNode *lChild = buildRandom();
            return new GNode(k, lChild);
          }
        case End:
          return new GNode(k);
      }
    }
  }
}

int main(int argc, char const *argv[]) {
  int seed = time(0);
  cout << "Seed = " << seed << "\n", srand(seed);

  // GNode *genome =
  //   GenSer(GenPar(GenIInc(GenICut(GenEnd())),
  // 		  GenICut(GenEnd())
  // 		 ),
  // 	   GenSer(GenPar(GenTInc(GenEnd()),
  // 			 GenEnd()
  // 			),
  // 		  GenWDec(GenEnd())
  // 		 )
  // 	   );
  builtNRandomNodes = 0;
  GNode *genome = buildRandom();

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

  INodes[0]->setValue(0);
  INodes[1]->setValue(0);
  INodes[2]->setValue(0);

  PNodes.push_back(new PNode(ONodes, INodes, 0, 0, genome, genome));

  Dump();

  bool isDone = false;
  for (size_t cycle = 0; !isDone; cycle += 1) {
    size_t hasMore = 0;
    size_t nPNodes = PNodes.size();

    for (size_t p = 0; p < nPNodes; p += 1) {
      PNode *pNode = PNodes[p];

      if (pNode->hasMore()) {
	cout << "# Growing by "
             << ::toString(pNode->getKind())
             << ": "
             << pNode->toString()
             << "\n";

	if (!pNode->Grow()) {
	  hasMore += 1;
	}
      }
    }

    cout << "# "
         << cycle
         << " ------------------------------------------------------------------------\n";
    Dump();

    isDone = 0 == hasMore && nPNodes == PNodes.size();
  }

  int o0 = ONodes[0]->Evaluate();
  int o1 = ONodes[1]->Evaluate();

  // cout << "{0,1,2} -> {" << o0 << "," << o1 << "}\n";
  // Dump();

  size_t p = 0;
  while (p < PNodes.size()) {
    if (!PNodes[p]->isEvaluated()) {
      if ((p + 1) < PNodes.size()) {
	std::swap(PNodes[p], PNodes.back());
      }
      delete PNodes.back();
      PNodes.pop_back();
    } else {
      p += 1;
    }
  }

  // int trials[][3] = {
  //   { -1, -1, -1 },
  //   { -1, -1,  0 },
  //   { -1, -1,  1 },
  //   { -1,  0, -1 },
  //   { -1,  0,  0 },
  //   { -1,  0,  1 },
  //   { -1,  1, -1 },
  //   { -1,  1,  0 },
  //   { -1,  1,  1 },
  //   {  0, -1, -1 },
  //   {  0, -1,  0 },
  //   {  0, -1,  1 },
  //   {  0,  0, -1 },
  //   {  0,  0,  0 },
  //   {  0,  0,  1 },
  //   {  0,  1, -1 },
  //   {  0,  1,  0 },
  //   {  0,  1,  1 },
  //   {  1, -1, -1 },
  //   {  1, -1,  0 },
  //   {  1, -1,  1 },
  //   {  1,  0, -1 },
  //   {  1,  0,  0 },
  //   {  1,  0,  1 },
  //   {  1,  1, -1 },
  //   {  1,  1,  0 },
  //   {  1,  1,  1 },
  // };
  // 
  // for (size_t t = 0; t < (sizeof(trials) / sizeof(trials[0])); t += 1) {
  //   // ONodes[0]->Reset();
  //   // ONodes[1]->Reset();
  // 
  //   for (auto &i : INodes) {
  //     i->Reset();
  //   }
  //   for (auto &o : ONodes) {
  //     o->Reset();
  //   }
  //   for (auto &p : PNodes) {
  //     p->Reset();
  //   }
  // 
  //   for (size_t i = 0; i < 3; i += 1) {
  //     INodes[i]->setValue(trials[t][i]);
  //   }
  // 
  //   int o0 = ONodes[0]->Evaluate();
  //   int o1 = ONodes[1]->Evaluate();
  // 
  //   Dump();
  // 
  //   cout << "{"
  //        << trials[t][0]
  //        << ","
  //        << trials[t][1]
  //        << ","
  //        << trials[t][2]
  //        << "} -> {"
  //        << o0
  //        << ","
  //        << o1
  //        << "}\n";
  // }

  for (size_t t = 0; t < 1000; t += 1) {
    for (auto &i : INodes) {
      i->Reset();
    }
    for (auto &o : ONodes) {
      o->Reset();
    }
    for (auto &p : PNodes) {
      p->Reset();
    }

    INodes[0]->setValue(rand() % 11 - 5);
    INodes[1]->setValue(rand() % 11 - 5);
    INodes[2]->setValue(rand() % 11 - 5);

    ONodes[0]->Reset();
    ONodes[1]->Reset();

    int o0 = ONodes[0]->Evaluate();
    int o1 = ONodes[1]->Evaluate();

    cout << "{" << INodes[0]->Evaluate()
         << "," << INodes[1]->Evaluate()
         << "," << INodes[2]->Evaluate()
         << "} -> {" << o0 << "," << o1 << "}\n";
  }

  Dump();

  return 0;
}
