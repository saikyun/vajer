(defn inc [x] (+ x 1))

(declare thing [[:AST] -> [:AST]])
(defn thing
  [args]
  (var l NULL)
  (var i 0)
  (while (< i (arrlen args))
    (do
      (arrpush l (in args i))
      (set i (inc i))))
  (arr_to_list l))

(defmacro other_list
  [args]
  (var l NULL)
  (var i 0)
  (while (< i (arrlen args))
    (do
      (arrpush l (in args i))
      (set i (inc i))))
  (arr_to_list l))