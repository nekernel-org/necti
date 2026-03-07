extern palloc;

const main() {
  let type := 0;
  let sz := 0;
  let align := 0;
  
  const ptr := palloc(type, sz, align);
  return 0;
}

