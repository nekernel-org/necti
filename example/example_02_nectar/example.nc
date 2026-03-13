extern exit;

trait foo {
   let exit();
};

let foo.exit() {
    exit(0);
}

const main() {
  let self := 0;
  foo.exit();
  return 0;
}
