;;
;; GNU-lisp functions for editing SM macro files
;;
;; The following should be put in your .emacs file to automatically
;; load these functions when editing files *.m or *.sav
;;
;;(setq auto-mode-alist
;;      (cons (cons "\\.m$" 'sm-mode)
;;	    (cons (cons "\\.sav$" 'sm-mode) auto-mode-alist)))
;;(autoload 'sm-mode "~rhl/sm/sm.el" nil t)
;;
;; Alternatively, if the first line of a file contains the string
;;		-*-SM-*-
;; emacs will load sm-mode (note the lower case sm).
;;
;; Of course, you'd usually want to have the first line of the macro file
;; start with ## in the first two columns.
;; 
(defvar SM-indent 3 "*Indentation for blocks.")
(defvar SM-indent-continue 4 "*Extra indentation for continuation lines.")
(defvar SM-comment-column 40 "*Comments start in this column")
(defvar SM-tab-always-indent nil
  "*Non-nil means TAB in SM mode should always reindent the current line,
regardless of where in the line point is when the TAB command is used.")

(defvar SM-mode-syntax-table nil
  "Syntax table in use in SM-mode buffers.")

(if SM-mode-syntax-table
    ()
  (setq SM-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?_ "w" SM-mode-syntax-table)
  (modify-syntax-entry ?. "w" SM-mode-syntax-table)
  (modify-syntax-entry ?+ "." SM-mode-syntax-table)
  (modify-syntax-entry ?- "." SM-mode-syntax-table)
  (modify-syntax-entry ?* "." SM-mode-syntax-table)
  (modify-syntax-entry ?/ "." SM-mode-syntax-table)
  (modify-syntax-entry ?% "." SM-mode-syntax-table)
  (modify-syntax-entry ?\" "\"" SM-mode-syntax-table)
  (modify-syntax-entry ?\\ "/" SM-mode-syntax-table)
  (modify-syntax-entry ?{ "(}" SM-mode-syntax-table)
  (modify-syntax-entry ?} "){" SM-mode-syntax-table)
  (modify-syntax-entry ?# "<" SM-mode-syntax-table)
  (modify-syntax-entry ?\n ">" SM-mode-syntax-table))

(defvar SM-mode-map () 
  "Keymap used in SM mode.")

(if SM-mode-map
    ()
  (setq SM-mode-map (make-sparse-keymap))
  (define-key SM-mode-map "\t" 'SM-tab)
  (define-key SM-mode-map "\C-m" 'SM-newline-and-indent)
  (define-key SM-mode-map "}" 'SM-insert-and-indent)
  (define-key SM-mode-map "\e\C-a" 'beginning-of-SM-macro)
  (define-key SM-mode-map "\e\C-e" 'end-of-SM-macro)
  (define-key SM-mode-map "\e\C-h" 'mark-SM-macro)
  (define-key SM-mode-map "\e\C-m" 'SM-indent-macro)
  (define-key SM-mode-map "\e\t" 'SM-indent-line)
  (define-key SM-mode-map "\e;" 'SM-add-comment)

(defun SM-mode () "alias for sm-mode for backwards compatibility"
  (interactive) (sm-mode))
;;
(defun sm-mode () "Major mode for editing SM code.
Lines starting anywhere but the left margin are indented by 2 tabs,
then all further indentation is done with spaces.

   Macro declarations are followed by a tab, then the number of args,
then another tab.

Variables controlling indentation style and extra features:

 SM-indent                                (default: 3)
    Indentation for blocks.
 SM-indent-continue                       (default: 4)
    Extra indentation for continuation lines.
 SM-comment-column                        (default: 40)
    Starting column for comments following code.
 SM-tab-always-indent                     (default: nil)
    Non-nil means TAB in SM mode should always reindent the current line,
    regardless of where in the line point is when the TAB command is used.

Useful user functions include:

  narrow-to-SM-macro
     Narrow buffer to current SM macro. The opposite is widen.
  next-narrow-SM-macro
     Skip to the next SM macro, and narrow to it. If the optional how-many
     argument is provided, skip forward that many macros first.
  SM-macro-name
     Print the name of the SM macro containing the cursor

Turning on SM mode calls the value of the variable SM-mode-hook 
with no args, if it is non-nil.
\\{SM-mode-map}"
  (interactive)
  (kill-all-local-variables)
  (set-syntax-table SM-mode-syntax-table)
  (make-local-variable 'indent-line-function)
  (setq indent-line-function 'SM-indent-line)
  (setq indent-tabs-mode nil)
  (make-local-variable 'comment-indent-function)
  (setq comment-indent-function nil)
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "[^\\\\]#+[ \t]*")
  (make-local-variable 'comment-start)
  (setq comment-start "# ")
  (setq comment-column SM-comment-column)
  (make-local-variable 'require-final-newline)
  (setq require-final-newline t)
  (use-local-map SM-mode-map)
  (setq mode-name "SM")
  (setq major-mode 'SM-mode)
  (run-hooks 'SM-mode-hook))

(defun SM-comment-hook ()
  (save-excursion
    (skip-chars-backward " \t")
    (max (+ 1 (current-column))
	 comment-column)
))

(defun beginning-of-SM-macro (num)
  "Moves point to the beginning of the current SM macro."
  (interactive "p")
  (while (not (= num 0))
    (setq num (- num 1))
    (re-search-backward "^[^ \t\n]" nil 'move)))

(defun end-of-SM-macro (num)
  "Moves point to the end of the current SM macro."
  (interactive "p")
  (while (not (= num 0))
    (setq num (- num 1))
    (end-of-line)
    (re-search-forward "^[^ \t\n]" nil 'move)
    (beginning-of-line)))

(defun mark-SM-macro ()
  "Put mark at end of SM macro, point at beginning. Marks are pushed."
  (interactive)
  (end-of-SM-macro 1)
  (push-mark (point))
  (beginning-of-SM-macro 1)
  (buffer-substring (point)
		  (save-excursion (skip-chars-forward "^ \t\n") (point))))
;;
;; Now definitions to narrow the buffer to the next/current/previous definition
;;
(defun narrow-to-SM-macro ()
  "Narrow buffer to current SM macro."
  (interactive)
  (if (looking-at "^[^ \t]") (forward-word 1))
  (let ( (end (save-excursion (end-of-SM-macro 1) (point)))
	 (start (save-excursion (beginning-of-SM-macro 1) (point))) )
    (narrow-to-region start end)
    (beginning-of-line)))
;;
(defun next-narrow-SM-macro (&optional how-many)
  "Skip to the next SM macro, and narrow to it. If the optional how-many
argument is provided, skip forward that many macros first."
  (interactive "p")
  (if (not how-many) (setq how-many 1))
  (widen)
  (if (not (= how-many 0))
      (progn
	(if (> how-many 0)
	    (end-of-SM-macro how-many)
	  (beginning-of-SM-macro (abs how-many)))
	(narrow-to-SM-macro))))
;;
(defun SM-macro-name ()
  "Print the name of the SM macro containing the cursor"
  (interactive)
  (save-excursion
    (if (not (looking-at "^[^ \t]"))
	(beginning-of-SM-macro 1))
    (let ( (start (point)) (name) (nargs) )
      (skip-chars-forward "^ \t")
      (setq name (buffer-substring start (point)))
      (skip-chars-forward " \t")
      (setq start (point))
      (setq nargs
	    (if (looking-at "[0-9]")
		(progn
		  (skip-chars-forward "0-9")
		  (buffer-substring start (point)))
	      "0"))
      (message (concat "Current macro is " name
		       (if (not (string-equal nargs "0"))
			   (format " (%s args)" nargs)))))))
      


(defun SM-indent-line ()
  "Indent current SM line based on its contents and on previous lines.
Deal with first lines of macros correctly."
  (interactive)
  (let ((cfi (calculate-SM-indent)))
    (save-excursion (SM-indent-to-column cfi)
      (beginning-of-line)
      (if (looking-at "[^ \t\n]")	; macro declaration
	  (progn
	    (skip-chars-forward "^ \t")
	    (cond ((looking-at "[ \t]*$") (delete-horizontal-space))
		  ((looking-at "[ \t]*[^ \t0-9]") ; No arguments
		   (if (< (current-column) 8)
		       (if (not (looking-at "\t\t[^ \t]"))
			   (progn
			     (delete-horizontal-space)
			     (insert "\t\t")))
		       (if (not (looking-at "\t[^ \t]"))
			   (progn
			     (delete-horizontal-space)
			     (insert "\t")))))
		  (t			; Macro has arguments
		   (if (< (current-column) 8)
		       (if (not (looking-at "\t[0-9]+\t"))
			   (progn
			     (delete-horizontal-space)
			     (insert "\t")
			     (skip-chars-forward "0-9")
			     (delete-horizontal-space)
			     (insert "\t")))
		       (if (not (looking-at " [0-9]+\t"))
			   (progn
			     (delete-horizontal-space)
			     (insert " ")
			     (skip-chars-forward "0-9")
			     (delete-horizontal-space)
			     (insert "\t")))))))))
	    (if (< (current-column) cfi) (move-to-column cfi))))

(defun SM-newline-and-indent ()
  "Insert a newline, and indent. When splitting lines, preserve indentation"
  (interactive)
  (if (looking-at "$")
      (newline-and-indent)
    (progn (newline) (insert " ") (SM-indent-line))))

(defun SM-indent-macro ()
  "Properly indents the SM macro which contains point."
  (interactive)
  (save-excursion
    (mark-SM-macro)
    (let ((name (mark-SM-macro)))
      (message (format "Indenting macro %s..." name))
      (indent-region (point) (mark) nil)
      (message (format "Indenting macro %s... done." name))))
  (let ((col (calculate-SM-indent)))
    (if (< (current-column) col)
	(move-to-column col)))
  )

(defun calculate-SM-indent ()
  "Calculates the SM indent column based on previous lines."
  (let (icol)
    (save-excursion
      (if (SM-previous-statement)
	  (setq icol 16)		; minimum indentation (2 tabs)
	(progn
	  (if (= (point) (point-min))
	      (setq icol 16)
	    (setq icol (SM-current-line-indentation))))))
    (save-excursion
      (beginning-of-line)
      (cond
       ((looking-at "[^ \t\n]") (setq icol 0))
       ((save-excursion
	  (forward-line -1)
	  (if (looking-at ".*\\\\$") (setq icol (+ icol SM-indent-continue)))))
       ((save-excursion
	  (skip-paired-braces)
	  (looking-at "[^#\n]*}")) (setq icol (- icol SM-indent)))
      )
      (if (looking-at "[ \t\n]")
	  (progn
	    (forward-line -1)
	    (skip-paired-braces)
	    (if (looking-at "[^#\n]*{")
		(setq icol (+ icol SM-indent))))))
      (max (if (= icol 0) 0 16) icol)))

(defun skip-paired-braces () "Skip pairs of braces {...}"
  (interactive)
  (while
      (re-search-forward "{[^#}]*}" (save-excursion (end-of-line) (point)) t)
    (goto-char (match-end 0))))

(defun SM-current-line-indentation ()
  "Indentation of current line. If it's zero, look back until we find a
non-empty line"
  (save-excursion
    (beginning-of-line)
    (while (and (> (point) (point-min)) (looking-at "^[ \t]*$"))
      (forward-line -1))
    ;; Move past whitespace.
    (skip-chars-forward " \t")
    (current-column)))

(defun SM-indent-to-column (col)
  "Indents current line with up to 2 tabs then spaces to column COL."
  (save-excursion
    (beginning-of-line)
    (if (not (and (progn (skip-chars-forward "\t ") (= (current-column) col))
	      (looking-at "\t\t")))
	(progn
	  (beginning-of-line)
	  (delete-horizontal-space)
	  (SM-indent-to col)))
	  (skip-chars-forward "\t ")
      ;; Indent any comment following code on the same line
	  (if (and
	       (not (looking-at comment-start))
;	       (save-excursion
;		 (beginning-of-line) (looking-at "[ \t\n]"))
	       (re-search-forward comment-start-skip
				  (save-excursion (end-of-line) (point)) t))
	      (progn (goto-char (match-beginning 0))
		     (if (not (= (+ 1 (current-column)) (SM-comment-hook)))
			 (progn
			   (delete-horizontal-space)
			   (SM-indent-to (SM-comment-hook))))))))

(defun SM-indent-to (arg)
  "Like indent-to, but use tabs for first 16 columns"
   (interactive)
   (indent-to arg)
   (save-excursion
     (beginning-of-line)
     (if (looking-at "                ") ; 16 spaces
         (progn (delete-char 16) (insert "\t\t")))))

(defun SM-previous-statement ()
  "Moves point to beginning of the previous SM statement.
Returns 'first-statement if that statement is the first
non-comment SM statement in the file, and nil otherwise.
Skip to first line of multiple line-statements (ending in \\)."
  (interactive)
  (cond ((or
	  (not (= (forward-line -1) 0))
	  (looking-at "[^ \t\n]"))
	      'first-statement)
	(t
	 (beginning-of-line)
	 (while (and (or (looking-at "[ \t]*$") (looking-at ".*\\\\$"))
		     (= (forward-line -1) 0)
		     ))
	 nil
	 )))

(defun SM-add-comment ()
  (interactive)
  (beginning-of-line)
  (if (re-search-forward comment-start-skip
		     (save-excursion (end-of-line) (point)) t)
      (progn (if (not (=
	    (progn (goto-char (+ 1 (match-beginning 0)))
		   (current-column)) (SM-comment-hook)))
	  (progn
	    (delete-horizontal-space)
	    (SM-indent-to (SM-comment-hook))))
	     (if (looking-at "#* ")	; move to start of text
		 (skip-chars-forward "# ")
	       (progn (end-of-line) (insert " "))))
    (progn
      (end-of-line)
      (delete-horizontal-space)
      (SM-indent-to (SM-comment-hook))
      (insert comment-start))))

(defun SM-insert-and-indent ()
  "Insert a key, and indent the line"
  (interactive)
  (insert last-command-char)
  (SM-indent-line)
  (blink-matching-open))

(defun SM-tab (a)
  "With a prefix argument simply insert a tab (or spaces as appropriate).
If SM-tab-always-indent is nil indent the current line if dot is to the
left of any text, otherwise insert an (SM) tab. If SM-tab-always-indent
is non-nil, simply reindent line"
  (interactive "p")
;  (debug)
  (if (not (= 1 a))			; insert an (SM) tab
      (SM-insert-tab)
    (if SM-tab-always-indent
	(SM-indent-line)
      (let ((before-all-text
	     (save-excursion (skip-chars-backward " \t") (looking-at "^"))))
	(if before-all-text
	    (SM-indent-line)
	  (SM-insert-tab))))))
  
(defun SM-insert-tab ()
  "insert spaces to the next tab stop, except at the start of a line where
up to two real tabs are allowed"
  (interactive)
  (if (or (looking-at "^") (save-excursion (backward-char) (looking-at "^")))
	  (insert ?\C-i) (insert-tab)))
)
