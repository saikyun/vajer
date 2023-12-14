(declare malloc [:int -> :void*])

(var symbols (malloc (* 5 (sizeof :char*))))