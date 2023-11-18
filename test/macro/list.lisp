(defn inc [x] (+ x 1))

(defmacro list
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

(defn main
  []
  (printf "adder: %d\n" (list + 1 2))
  (assert (== 3 (list + 1 2)))
  (assert (== (list + 3 4) (list + 3 4)))
  (assert (== 7 (other_list + 3 4)))
  0)
