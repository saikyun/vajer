(defn add1 [x y]
  (var xx x)
  (printf "xx: %d\n" xx))

(defn add2 [x y]
  (var x x)
  (var xx x)
  (printf "xx: %d\n" xx))

(defn main []
  (add1 5 6)
  (add2 7 8))