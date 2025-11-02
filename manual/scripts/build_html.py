'''
build_html.py
使用 material for mkdocs 构建在线版本文档
使用 pandoc 将 typst 转换为 md，然后使用 mkdocs 构建
需要在 manual 目录下使用 python ./scripts/build_html.py 调用
构建后的文档在 manual/site 下
'''
import os

for file in os.listdir('docs'):
    if file.endswith('.typ') and file != 'lib.typ' and file != 'manual.typ':
        new_file = file.replace('.typ', '.md')
        os.system(f'cd docs && pandoc {file} -o {new_file}')

os.system('mkdocs build -v')
