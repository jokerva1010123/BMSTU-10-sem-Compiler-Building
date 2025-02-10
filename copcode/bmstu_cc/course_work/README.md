# 🌿 КУРСОВОЙ ПРОЕКТ 
## "Разработка компилятора языка TinyC"
Компилятор: <br/>
✓ написан на языке Python; <br/>
✓ использует грамматику языка C, дальнейшая обработка которой адаптирована под язык TinyC; <br/>
✓ использует ANTLR в качестве генератора лексического и синтаксического анализаторов; <br/>
✓ использует LLVM в качестве генератора машинного кода <br/>
<br/>
## РАБОТА С ПРОЕКТОМ 
1) Генерация .ll файла, содержащего LLVM IR соответствующего .tc файла:
```
python  llvm_ir.py  path_to_TinyC_file
```
2) Запуск сгенерированного .ll файла с помощью llvmlite (LLVM):
```
python  executor.py  path_to_ll_file
```

> [!NOTE]
> С помощью запуска файла *auto_start.py* можно автоматизировать выполнение двух предыдущих команд

<br/>

## 🍃 *Might be useful* 🍃
p.s. при написании компилятора любого языка на Python <br/>
<br/>
**1) установка ANTLR:**
```
pip install antlr4-python3-runtime
pip install antlr4-tools
```

**2) установка LLVM:**
```
pip install llvmlite
```

**3) генерация Parser, Lexer, Listener и Visitor (с помощью ANTLR):**
```
antlr4 -Dlanguage=Python3 antlr_parser/TinyC.g4 -o antlr_parser -visitor
```
> [!IMPORTANT]
> ▫️ На основе автоматически сгенерированного класса TinyCVisitor необходимо создать *TinyCGenerator* -- класс-наследник, внутри которого будут переопределяться соответствующие методы класса TinyCVisitor <br/>
> ▫️ Впоследствии реализация *TinyCGenerator* должна позволять производить перевод построенного AST (Abstract Syntax Tree) в LLVM IR (Intermediate Representation)
