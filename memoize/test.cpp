

__attribute__((annotate("memoize"))) int fac(int x) {
  return x == 0 ? 1 : x * fac(x - 1);
}

int main() { return fac(5); }
