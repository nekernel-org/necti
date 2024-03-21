
export .text foo
  pha
  lda r2, 0x1000
  mv r19, r2
  jrl
  pla
  jlr
