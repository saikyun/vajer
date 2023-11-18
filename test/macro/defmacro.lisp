(defn inc
  [x]
  (+ x 1))

(defmacro list
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
  (var x 10)
  (when (= x 0)
    (printf "hello" "")
    (printf "yeah" "")))