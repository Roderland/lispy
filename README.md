def {fun} (\ {args body} {def (head args) (\ (tail args) body)})

(fun {len l} { if (== l {}) {0} {+ 1 (len (tail l))}})