# HCML-(X)

Introducing the HCML-(X) specification, that add new tools to the HCML specification.

## Basic HCML

### `<?load>`

The `<?load>` tag is used to load a file into the current file. The file is loaded as if it was written in the current file.

```hcml
</?load file="file.md">
```
The `<?load>` is replaced in the html structue by the root tag of the file.

### `<?set>`

The `<?set>` tag is used to set a variable in the current file. The variable will be set to the tag html. This means it will include the childrens inside the tag.

```hcml
<?set id="name" type="div">
    ... value ...
</?set>
```

##### Attributes

 - `id` : The name of the variable. It will be set as is to the value of the tag.
 - `type` : The type of the variable. It will define the name of the root tag of the variable. It is optional and default to `div`. Usually it's an html compliant name.

### `<?get>`

The `<?get>` tag is used to get a variable in the current file. The variable is get as if it was written in the current file. 

It can be used a self closing tag to get the value of the variable. The variable will replace the tag.

##### Simple example

**pre-compile:**
```hcml
</?get id="name">
```
**post-compile:**
```html
<div> ... value ... </div>
```
 - `div` is the default type of the variable, but it can be anything set in the `type` attribute of the `<?set>` tag.


##### Field specific example

The `<?get>` tag can also be used to get a field of a variable.

**pre-compile:**
```html
<?get id="name">
    <field>
    </field>
</?get>
```
**post-compile:**
```html
<field>
    ... value ...
</field>
```

In this case, the `<?get>` tag is replaced by the value of the field of the variable.

##### Multiple fields example

The `<?get>` tag can also be used to get multiple fields of a variable.

**pre-compile:**
```html
<?get name="name">
    <field1/>
    <field2/>
</?get>
```
**post-compile:**
```html
<field1>
    ... value ...
</field1>
<field2>
    ... value ...
</field2>
```



In this case, the `<?get>` tag is replaced by the values of the fields of the variable.

Its can also be used to get multiple nested fields of a variable.

**pre-compile:**
```html
<?get name="name">
    <field1>
        <field2>
    </field1>
</?get>
```
**post-compile:**
```html
<field2>
    ... content ...
</field2>
```

### `<?if>`
The `<?if>` tag is used to conditionaly include a part of the file. The tag can have a `<?then>` and a `<?else>` tag. The `<?then>` tag is included if the condition is true, and the `<?else>` tag is included if the condition is false. The `true` and `false` values are defined by the `cond` attribute of the `<?if>` tag.

##### Simple example

We can use the `<?if>` tag to include basic HTML code.

**pre-compile:**
```html
<?if cond="true">
    <p>true</p>
</?if>
<?then>
    <h1> Hello </h1>
</?then>
<?else>
    <h1> Goodbye </h1>
</?else>
```

**post-compile:**
```html
<h1>Hello</h1>
```

##### Variable example

We can use the `<?if>` tag to include a variable.

**pre-compile:**
```html
<?if cond="john">
    <?get id="name"/>
</?if>
<?then>
    <h1> Hello John </h1>
</?then>
<?else>
    <h1> I don't know you </h1>
</?else>
```

**post-compile:**

 - If the name variable is set to `john`:
```html
<h1>Hello John</h1>
```
- If the name variable is set to `jane`:
```html
<h1>I don't know you</h1>
```

### `<?eval>`

The `<?eval>` tag is used to evaluate a mathematical expression. The expression is defined in the `cond` attribute of the tag. The result of the evaluation is included in the file.

##### Simple example

We can use the `<?eval>` tag to include the result of a mathematical expression.

**pre-compile:**
```html
<?eval cond="2+2">
```

**post-compile:**
```html
4
```

### `<?fn>`

The `<?fn>` tag is used to define a function in the current file. The function can be called with the `<?call>` tag. The function will return it's content, in the form of html code after it's evaluation by thh HCML compiler. A function can take arguments. The arguments are passed as a variable and they are accessed with the `<?get>` tag. The argument are the body of the `<?call>` tag.  The `cond` supports logical operations.

##### Simple example

We can use the `<?fn>` tag to define a function that returns a value. 

**pre-compile:**
```html
<?fn id="hello">
    <p>Hello</p>
</?fn>
<?call id="hello"/>
```
**post-compile:**
```html
<p>Hello</p>
```
##### More complex example

**pre-compile:**
```html
<?fn id="hello">
    <p>Hello <?get></name></?get></p>
</?fn>

<?call id="hello">
    <name>
        John
    </name>
</?call>
```

**post-compile:**
```html
<p>Hello John</p>
```

##### Example with logic

**pre-compile:**
```html
<?fn id="is_logged_in">
    <?if cond="john|mary">
        <?get></name></?get>
    </?if>
    <?then>
        <p>Hi <?get></name></?get></p>
    </?then>
    <?else>
        <p>Not logged in</p>
        <p>We onmly like john or mary</p>
    </?else>
</?fn>
```

**post-compile:**

 - If the name variable is set to `john`:
```html
<p>Hi John</p>
```

- If the name variable is set to `jane`:
```html
<p>Not logged in</p>
<p>We only like john or mary</p>
```


## HCM(X)

Here is the list of the new tags that are added to the HCML specification for integration with the HTMX library.

### <?hx-on>

The `<?hx-on>` tag is used to handle the result of a parent HTMX request. The tag containes server code that is executed when the parent request is done.

```html
<div hx-post="/endpoint">
    <?hx-on>
        <?if cond="john">
            <?get>
                </name>
            </?get>
        </?if>
        <?then>
            <p>Welcome <?get></name></?get> </p>
        <?else>
                <p>Not logged in</p>
        </?then>
    <?/hx-on>
</div>
```
We need to keep in mind that the cond inside the <?hx-on> is executed on the server and doesnt have access to the client side variables. In order to pass the client side variables to the server, we need to use the `pass` attribute of the `hx-on` tag.

```html
<div hx-post="/endpoint">
    <?hx-on pass="name">
        <?if>
            <?fn id="is_logged_in">
                </?get name="name">
            </?fn>
        </?if>
        <?then>
            <?return>
                <p>Welcome <?get></name></?get> </p>
            </?return>
        <?else>
            <?return>
                <p>Not logged in</p>
            </?return>
        </?then>
    <?/hx-on>
</div>
```