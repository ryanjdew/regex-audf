#include <MarkLogic.h>
#include <regex.h> 
#include <vector>

#ifdef _MSC_VER
#define PLUGIN_DLL __declspec(dllexport)
#else // !_MSC_VER
#define PLUGIN_DLL
#endif

using namespace marklogic;

////////////////////////////////////////////////////////////////////////////////

class Regex : public AggregateUDF
{
public:
  String regex;
  regex_t regex_compiled;
  std::vector<String*> matches;
public:
  AggregateUDF* clone() const { return new Regex(*this); }
  void close();

  void start(Sequence&, Reporter&);
  void finish(OutputSequence& os, Reporter& reporter);

  void map(TupleIterator& values, Reporter& reporter);
  void reduce(const AggregateUDF* _o, Reporter& reporter);

  void encode(Encoder& e, Reporter& reporter);
  void decode(Decoder& d, Reporter& reporter);

};

void Regex::
close() { 
  matches.clear();
  delete this; 
}

void Regex::
start(Sequence& arg, Reporter& reporter)
{
  int reti;
  arg.value(regex);
  reporter.log(Reporter::Info, regex.get());
  /* Compile regular expression */
  reti = regcomp(&regex_compiled, regex.get(), 0);
}

void Regex::
finish(OutputSequence& os, Reporter& reporter)
{
  /* Free compiled regular expression if you want to use the regex_t again */
  regfree(&regex_compiled);
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
  {
    os.writeValue(*(matches[i]));
  }
}

void Regex::
map(TupleIterator& values, Reporter& reporter)
{
  int reti = 0;
  for(; !values.done(); values.next()) {
    if(!values.null(0)) {
      String cur; values.value(0,cur);
	  /* Execute regular expression */
	  reti = regexec(&regex_compiled, cur.get(), 0, NULL, 0);
	  if( !reti ){
		  reporter.log(Reporter::Info, "Match found.");
		  matches.push_back(&cur);
	  }
    }
  }
}

void Regex::
reduce(const AggregateUDF* _o, Reporter& reporter)
{
  return;
  const Regex *o = (const Regex*)_o;
  int o_size = o->matches.size();
  for (int i = 0; i < o_size; i++) {
    matches.push_back(o->matches[i]);
  }
}

void Regex::
encode(Encoder& e, Reporter& reporter)
{
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
    e.encode(*(matches[i]),sizeof(String));
}

void Regex::
decode(Decoder& d, Reporter& reporter)
{
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
    d.decode((matches[i]),sizeof(String));
}

////////////////////////////////////////////////////////////////////////////////
/*
class ReverseRegex : public AggregateUDF
{
public:
  double value;

public:
  AggregateUDF* clone() const { return new ReverseRegex(*this); }
  void close() { delete this; }

  void start(Sequence&, Reporter&);
  void finish(OutputSequence& os, Reporter& reporter);

  void map(TupleIterator& values, Reporter& reporter);
  void reduce(const AggregateUDF* _o, Reporter& reporter);

  void encode(Encoder& e, Reporter& reporter);
  void decode(Decoder& d, Reporter& reporter);
};

void ReverseRegex::
start(Sequence& arg, Reporter& reporter)
{
  arg.value(value);
}

void ReverseRegex::
finish(OutputSequence& os, Reporter& reporter)
{
  if(!empty) os.writeValue(max);
}

void ReverseRegex::
map(TupleIterator& values, Reporter& reporter)
{
  for(; !values.done(); values.next()) {
    if(!values.null(0)) {
      double v; values.value(0,v);
      if(v > value) gt += values.frequency();
      else if(v == value) eq += values.frequency();
      else lt += values.frequency();
    }
  }
}

void ReverseRegex::
reduce(const AggregateUDF* _o, Reporter& reporter)
{
  const MedianTest *o = (const MedianTest*)_o;
  gt += o->gt;
  eq += o->eq;
  lt += o->lt;
}

void ReverseRegex::
encode(Encoder& e, Reporter& reporter)
{
  e.encode(value);
  e.encode(gt);
  e.encode(eq);
  e.encode(lt);
}

void ReverseRegex::
decode(Decoder& d, Reporter& reporter)
{
  d.decode(value);
  d.decode(gt);
  d.decode(eq);
  d.decode(lt);
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
class Max : public AggregateUDF
{
public:
  bool empty;
  T max;
  bool dolt;
  T lt;

public:
  Max(): empty(true), max(), dolt(false), lt() {}

  AggregateUDF* clone() const { return new Max(*this); }
  void close() { delete this; }

  void start(Sequence&, Reporter&);
  void finish(OutputSequence& os, Reporter& reporter);

  void map(TupleIterator& values, Reporter& reporter);
  void reduce(const AggregateUDF* _o, Reporter& reporter);

  void encode(Encoder& e, Reporter& reporter);
  void decode(Decoder& d, Reporter& reporter);

  RangeIndex::Order getOrder() const { return RangeIndex::DESCENDING; }
};

template<typename T> void Max<T>::
start(Sequence& arg, Reporter& reporter)
{
  if(!arg.done()) { dolt=true; arg.value(lt); }
}

template<typename T> void Max<T>::
finish(OutputSequence& os, Reporter& reporter)
{
  if(!empty) os.writeValue(max);
}

template<typename T> void Max<T>::
map(TupleIterator& values, Reporter& reporter)
{
  for(; !values.done(); values.next()) {
    if(values.null(0)) continue;
    T tmp; values.value(0,tmp);
    if(!dolt || tmp < lt) { empty = false; max = tmp; break; }
  }
}

template<typename T> void Max<T>::
reduce(const AggregateUDF* _o, Reporter& reporter)
{
  const Max *o = (const Max*)_o;
  if(empty || (!o->empty && o->max > max)) {
    empty = o->empty;
    max = o->max;
  }
}

template<typename T> void Max<T>::
encode(Encoder& e, Reporter& reporter)
{
  e.encode(empty);
  if(!empty) e.encode(max);
  e.encode(dolt);
  if(dolt) e.encode(lt);
}

template<typename T> void Max<T>::
decode(Decoder& d, Reporter& reporter)
{
  d.decode(empty);
  if(!empty) d.decode(max);
  d.decode(dolt);
  if(dolt) d.decode(lt);
}
*/
////////////////////////////////////////////////////////////////////////////////

extern "C" PLUGIN_DLL void
marklogicPlugin(Registry& r)
{
  r.version();
  r.registerAggregate<Regex>("regex");
}
