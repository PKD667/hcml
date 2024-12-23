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
The `<?if>` tag is used to conditionaly include a part of the file. The tag can have an `<?else>` tag. The content of the `<?if>` tag is included if the condition is true, and the `<?else>` tag is included if the condition is false. The condition is defined in the `cond` attribute of the `<?if>` tag. The `cond` supports logical operations.

##### Simple example

**pre-compile:**
```html
<?if cond="true">
    <p>Hello</p>
</?if>
```

**post-compile:**
```html
<p>Hello</p>
```

##### More complex example

**pre-compile:**
```html
<?set id="name">
    John
</?set>

<?if cond="name == 'John'">
    <p>Hello John</p>
</?if>
<?else>
    <p>Hello Someone else</p>
</?else>
```

**post-compile:**
```html
<p>Hello John</p>
```

### `<?call>`

The `<?call>` tag is used to call an extern function. The function is defined in the `id` attribute of the `<?call>` tag. The function arguments are its childrens.

```html
<?call id="function">
    <arg1>
        value1
    </arg1>
    <arg2>
        value2
    </arg2>
</?call>
```

## HCM(X)

Here is the list of the new tags that are added to the HCML specification for integration with the HTMX library.

### <?hx-on>

The `<?hx-on>` tag is used to handle the result of a parent HTMX request. The tag containes server code that is executed when the parent request is done.

```html
<div hx-post="/endpoint">
    <?hx-on>
        <?if cond="request#name = 'John'">
            <p>Welcome <?get id="request"><name/></?get> </p>
        </?if>   
        <?else>
                <p>Not logged in</p>
        </?else>
    <?/hx-on>
</div>
```
We need to keep in mind that the cond inside the <?hx-on> is executed on the server and doesnt have access to the client side variables. 
```html
<div hx-post="/endpoint">
    <?hx-on>
        <?if>
            <?call id="is_logged_in">
                <?get id="request">
                    <name/>
                </?get>
            </?fn>
        </?if>
        <?then>
            <p>Welcome <?get id="request"><name/></?get> </p>
        <?else>
            <p>Not logged in</p>
        </?then>
    <?/hx-on>
</div>
```