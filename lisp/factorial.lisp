(defn fac [n]
  (if (<= n 1)
    1
    (* n (factorial (- n 1)))
  )
)