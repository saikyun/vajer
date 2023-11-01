(declare printf [[:char] -> :void])

(var a 10)

(defn pront
  [a]
  (printf a)
  )

(defn main
  []
  (pront "hello\n")
  0)