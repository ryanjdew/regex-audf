#include <MarkLogic.h>
#include <regex.h> 
#include <string.h>
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
  /* Compile regular expression */
  reti = regcomp(&regex_compiled, regex.get(), 0);
}

void Regex::
finish(OutputSequence& os, Reporter& reporter)
{
  /* Free compiled regular expression */
  regfree(&regex_compiled);
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
  {
    os.writeValue(*(matches[i]));
	/* Clean up created strings */
    delete matches[i];
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
		  /* If matches then create a copy of the string */
		  size_t str_length = strlen(cur.get());
		  char cp_str[str_length];
		  strcpy(cp_str,cur.get());
		  /* Store the pointer to the marklogic::String for output later */
		  matches.push_back(new String(cp_str,cur.collation()));
	  }
    }
  }
}

void Regex::
reduce(const AggregateUDF* _o, Reporter& reporter)
{
  /* Merge matches found */
  const Regex *o = (const Regex*)_o;
  std::vector<String*> o_matches = o->matches;
  matches.insert(matches.end(),o_matches.begin(),o_matches.end());
}

void Regex::
encode(Encoder& e, Reporter& reporter)
{
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
    e.encode(matches[i],sizeof(String));
}

void Regex::
decode(Decoder& d, Reporter& reporter)
{
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
    d.decode(matches[i],sizeof(String));
}

////////////////////////////////////////////////////////////////////////////////
class ReverseRegex : public AggregateUDF
{
public:
  String rregex;
  regex_t regex_compiled;
  std::vector<String*> matches;
public:
  AggregateUDF* clone() const { return new ReverseRegex(*this); }
  void close();

  void start(Sequence&, Reporter&);
  void finish(OutputSequence& os, Reporter& reporter);

  void map(TupleIterator& values, Reporter& reporter);
  void reduce(const AggregateUDF* _o, Reporter& reporter);

  void encode(Encoder& e, Reporter& reporter);
  void decode(Decoder& d, Reporter& reporter);

};

void ReverseRegex::
close() { 
  matches.clear();
  delete this; 
}

void ReverseRegex::
start(Sequence& arg, Reporter& reporter)
{
  arg.value(rregex);
}

void ReverseRegex::
finish(OutputSequence& os, Reporter& reporter)
{
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
  {
    os.writeValue(*(matches[i]));
	/* Clean up created strings */
    delete matches[i];
  }
}

void ReverseRegex::
map(TupleIterator& values, Reporter& reporter)
{
  for(; !values.done(); values.next()) {
    if(!values.null(0)) {
      String cur; values.value(0,cur);
	  const char *str_char = cur.get();
	  /* Compile regular expression */
	  if ( !regcomp(&regex_compiled, str_char, 0)) {
	    /* Execute regular expression */
	    if( !regexec(&regex_compiled, rregex.get(), 0, NULL, 0) ){
		  /* If matches then create a copy of the string */
		  size_t str_length = strlen(str_char);
		  char cp_str[str_length];
		  strcpy(cp_str,str_char);
		  /* Store the pointer to the marklogic::String for output later */
		  matches.push_back(new String(cp_str,cur.collation()));
	    }
	    /* Free compiled regular expression if you want to use the regex_t again */
	    regfree(&regex_compiled);
	  } 
    }
  }
}

void ReverseRegex::
reduce(const AggregateUDF* _o, Reporter& reporter)
{
  /* Merge matches found */
  const ReverseRegex *o = (const ReverseRegex*)_o;
  std::vector<String*> o_matches = o->matches;
  matches.insert(matches.end(),o_matches.begin(),o_matches.end());
}

void ReverseRegex::
encode(Encoder& e, Reporter& reporter)
{
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
    e.encode(matches[i],sizeof(String));
}

void ReverseRegex::
decode(Decoder& d, Reporter& reporter)
{
  int o_size = matches.size();
  for (int i = 0; i < o_size; i++)
    d.decode(matches[i],sizeof(String));
}
////////////////////////////////////////////////////////////////////////////////

extern "C" PLUGIN_DLL void
marklogicPlugin(Registry& r)
{
  r.version();
  r.registerAggregate<Regex>("regex");
  r.registerAggregate<ReverseRegex>("reverse-regex");
}
