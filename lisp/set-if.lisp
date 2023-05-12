(defn inc [x :int] :int
  (+ x 1))

(defn main [] :int
  (var x :int (inc (if 1 1 0)))

  #(var res :int
  #  (if (== 1 1)
  #    1337))
  0)