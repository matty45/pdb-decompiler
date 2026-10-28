enum Mode { First = 1, Second = 2 };
struct Sample { int value; Mode mode; int add(int amount); };
typedef unsigned long LegacyWord;
Sample sample;
LegacyWord global_count = 7;
int Sample::add(int amount) { int local = value + amount; value = local; return local; }
extern "C" int entry() { return sample.add(global_count); }
