#include <deque>
#include <map>
#include <unordered_map>

#include "boost/ut.hpp"
#include "glaze/glaze.hpp"

using namespace boost::ut;

struct my_struct
{
   int i{};
   double d{};
   std::string hello{};
   std::array<uint64_t, 3> arr{};
};

static_assert(glz::detail::reflectable<my_struct>);

suite reflection = [] {
   "reflect_write"_test = [] {
      std::string buffer = R"({"i":287,"d":3.14,"hello":"Hello World","arr":[1,2,3]})";
      my_struct obj{};
      expect(!glz::read_json(obj, buffer));

      expect(obj.i == 287);
      expect(obj.d == 3.14);
      expect(obj.hello == "Hello World");
      expect(obj.arr == std::array<uint64_t, 3>{1, 2, 3});

      buffer.clear();
      glz::write_json(obj, buffer);

      expect(buffer == R"({"i":287,"d":3.14,"hello":"Hello World","arr":[1,2,3]})");
   };
};

struct non_default_t
{
   non_default_t(int) {}
};

struct nested_t
{
   std::optional<std::string> str{};
   my_struct thing{};
};

static_assert(glz::detail::reflectable<nested_t>);

suite nested_reflection = [] {
   "nested_reflection"_test = [] {
      std::string buffer = R"({"thing":{"i":287,"d":3.14,"hello":"Hello World","arr":[1,2,3]},"str":"reflection"})";
      nested_t obj{};
      expect(!glz::read_json(obj, buffer));

      expect(obj.thing.i == 287);
      expect(obj.thing.d == 3.14);
      expect(obj.thing.hello == "Hello World");
      expect(obj.thing.arr == std::array<uint64_t, 3>{1, 2, 3});

      buffer.clear();
      glz::write_json(obj, buffer);

      expect(buffer == R"({"str":"reflection","thing":{"i":287,"d":3.14,"hello":"Hello World","arr":[1,2,3]}})")
         << buffer;
   };
};

struct sub_thing
{
   double a{3.14};
   std::string b{"stuff"};
};

struct sub_thing2
{
   double a{3.14};
   std::string b{"stuff"};
   double c{999.342494903};
   double d{0.000000000001};
   double e{203082348402.1};
   float f{89.089f};
   double g{12380.00000013};
   double h{1000000.000001};
};

struct V3
{
   double x{3.14};
   double y{2.7};
   double z{6.5};

   bool operator==(const V3& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }
};

enum class Color { Red, Green, Blue };

template <>
struct glz::meta<Color>
{
   static constexpr std::string_view name = "Color";
   static constexpr auto value = enumerate("Red", Color::Red, //
                                           "Green", Color::Green, //
                                           "Blue", Color::Blue //
   );
};

struct var1_t
{
   double x{};
};

struct var2_t
{
   double y{};
};

struct Thing
{
   sub_thing thing{};
   std::array<sub_thing2, 1> thing2array{};
   V3 vec3{};
   std::array<std::string, 4> array = {"as\"df\\ghjkl", "pie", "42", "foo"};
   std::vector<V3> vector = {{9.0, 6.7, 3.1}, {}};
   int i{8};
   double d{2};
   bool b{};
   char c{'W'};
   Color color{Color::Green};
   std::vector<bool> vb = {true, false, false, true, true, true, true};
   std::optional<V3> optional{};
   sub_thing* thing_ptr{&thing};
};

suite user_types = [] {
   "complex user obect"_test = [] {
      Thing obj{};
      std::string buffer{};
      glz::write_json(obj, buffer);
      expect(
         buffer ==
         R"({"thing":{"a":3.14,"b":"stuff"},"thing2array":[{"a":3.14,"b":"stuff","c":999.342494903,"d":1E-12,"e":203082348402.1,"f":89.089,"g":12380.00000013,"h":1000000.000001}],"vec3":{"x":3.14,"y":2.7,"z":6.5},"array":["as\"df\\ghjkl","pie","42","foo"],"vector":[{"x":9,"y":6.7,"z":3.1},{"x":3.14,"y":2.7,"z":6.5}],"i":8,"d":2,"b":false,"c":"W","color":"Green","vb":[true,false,false,true,true,true,true],"thing_ptr":{"a":3.14,"b":"stuff"}})")
         << buffer;

      buffer.clear();
      glz::write<glz::opts{.skip_null_members = false}>(obj, buffer);
      expect(
         buffer ==
         R"({"thing":{"a":3.14,"b":"stuff"},"thing2array":[{"a":3.14,"b":"stuff","c":999.342494903,"d":1E-12,"e":203082348402.1,"f":89.089,"g":12380.00000013,"h":1000000.000001}],"vec3":{"x":3.14,"y":2.7,"z":6.5},"array":["as\"df\\ghjkl","pie","42","foo"],"vector":[{"x":9,"y":6.7,"z":3.1},{"x":3.14,"y":2.7,"z":6.5}],"i":8,"d":2,"b":false,"c":"W","color":"Green","vb":[true,false,false,true,true,true,true],"optional":null,"thing_ptr":{"a":3.14,"b":"stuff"}})")
         << buffer;

      expect(!glz::read_json(obj, buffer));
   };
};

int main()
{ // Explicitly run registered test suites and report errors
   // This prevents potential issues with thread local variables
   const auto result = boost::ut::cfg<>.run({.report_errors = true});
   return result;
}
